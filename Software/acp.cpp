#include "acp.h"

ACP::ACP()
{
}

ACP::ACP(InputArray _data, InputArray __mean, int flags, int maxComponents)
{
		monotonic_clock::time_point t1;
		monotonic_clock::time_point t2;
		duration<double> time_span;

		Mat data = _data.getMat(), _mean = __mean.getMat();
		int covar_flags = CV_COVAR_SCALE;
		int i, len, in_count;
		Size mean_sz;

		CV_Assert( data.channels() == 1 );
		if( flags & CV_PCA_DATA_AS_COL )
		{
			len = data.rows;
			in_count = data.cols;
			covar_flags |= CV_COVAR_COLS;
			mean_sz = Size(1, len);
		}
		else
		{
			len = data.cols;
			in_count = data.rows;
			covar_flags |= CV_COVAR_ROWS;
			mean_sz = Size(len, 1);
		}

		int count = std::min(len, in_count), out_count = count;
		if( maxComponents > 0 )
			out_count = std::min(count, maxComponents);

		// "scrambled" way to compute PCA (when cols(A)>rows(A)):
		// B = A'A; B*x=b*x; C = AA'; C*y=c*y -> AA'*y=c*y -> A'A*(A'*y)=c*(A'*y) -> c = b, x=A'*y
		if( len <= in_count )
			covar_flags |= CV_COVAR_NORMAL;
		
		//calcula media---------------------------------------------------------
		int ctype = std::max(CV_32F, data.depth());
		t1 = monotonic_clock::now();
		mean.create( mean_sz, ctype );
		t2 = monotonic_clock::now();
		time_span = duration_cast<duration<double>>(t2 - t1);
		cout << "\t\t\t\tmean took " << time_span.count() << " seconds.\n";
		//----------------------------------------------------------------------
		Mat covar( count, count, ctype );

		if( _mean.data )
		{
			CV_Assert( _mean.size() == mean_sz );
			_mean.convertTo(mean, ctype);
			covar_flags |= CV_COVAR_USE_AVG;
		}

		//calcula matriz de coovariancia----------------------------------------
		cout<<"\t\t\t\tstart MatCovar----------------------------------"<<endl;
		t1 = monotonic_clock::now();
		calcCovarMatrix(data, covar, mean, covar_flags, ctype);
		t2 = monotonic_clock::now();
		time_span = duration_cast<duration<double>>(t2 - t1);
		cout << "\t\t\t\tcalcCovar took " << time_span.count() << " seconds.\n";
		//----------------------------------------------------------------------

		//calcula auto vetore e auto valor--------------------------------------
		cout<<"\t\t\t\tstart eigen----------------------------------"<<endl;
		t1 = monotonic_clock::now();
		eigen(covar, eigenvalues, eigenvectors);
		t2 = monotonic_clock::now();
		time_span = duration_cast<duration<double>>(t2 - t1);
		cout << "\t\t\t\teigen took " << time_span.count() << " seconds.\n";
		//----------------------------------------------------------------------

		//normalization---------------------------------------------------------
		cout<<"\t\t\t\tstart norm----------------------------------"<<endl;
		t1 = monotonic_clock::now();
		if( !(covar_flags & CV_COVAR_NORMAL) ){
			//CV_PCA_DATA_AS_ROW: cols(A)>rows(A). x=A'*y -> x'=y'*A
			//CV_PCA_DATA_AS_COL: rows(A)>cols(A). x=A''*y -> x'=y'*A'
			Mat tmp_data, tmp_mean = repeat(mean, data.rows/mean.rows, data.cols/mean.cols);
			if(data.type() != ctype || tmp_mean.data == mean.data){
				data.convertTo( tmp_data, ctype );
				subtract(tmp_data, tmp_mean, tmp_data);
			}else{
				subtract(data, tmp_mean, tmp_mean);
				tmp_data = tmp_mean;
			}

			Mat evects1(count, len, ctype);
			gemm(eigenvectors, tmp_data, 1, Mat(), 0, evects1, (flags & CV_PCA_DATA_AS_COL) ? CV_GEMM_B_T : 0);
			eigenvectors = evects1;

			//normalize eigenvectors
			for(i = 0; i < out_count; i++) {
				Mat vec = eigenvectors.row(i);
				normalize(vec, vec);
			}
		}
		t2 = monotonic_clock::now();
		time_span = duration_cast<duration<double>>(t2 - t1);
		cout << "\t\t\t\tnorm took " << time_span.count() << " seconds.\n";
		
		if( count > out_count ){
			//use clone() to physically copy the data and thus deallocate the original matrices
			eigenvalues = eigenvalues.rowRange(0,out_count).clone();
			eigenvectors = eigenvectors.rowRange(0,out_count).clone();
		}
		//return *this;
}

template <typename T>
int computeCumulativeEnergy(const Mat& eigenvalues, double retainedVariance)
	{
	CV_DbgAssert( eigenvalues.type() == DataType<T>::type );

	Mat g(eigenvalues.size(), DataType<T>::type);

	for(int ig = 0; ig < g.rows; ig++)
	{
		g.at<T>(ig, 0) = 0;
		for(int im = 0; im <= ig; im++)
		{
			g.at<T>(ig,0) += eigenvalues.at<T>(im,0);
		}
	}

	int L;

	for(L = 0; L < eigenvalues.rows; L++)
	{
		double energy = g.at<T>(L, 0) / g.at<T>(g.rows - 1, 0);
		if(energy > retainedVariance)
			break;
	}

	L = std::max(2, L);

	return L;
}

ACP::ACP(InputArray _data, InputArray __mean, int flags, double retainedVariance){
	Mat data = _data.getMat(), _mean = __mean.getMat();
	int covar_flags = CV_COVAR_SCALE;
	int i, len, in_count;
	Size mean_sz;

	CV_Assert( data.channels() == 1 );
	if( flags & CV_PCA_DATA_AS_COL )
	{
		len = data.rows;
		in_count = data.cols;
		covar_flags |= CV_COVAR_COLS;
		mean_sz = Size(1, len);
	}
	else
	{
		len = data.cols;
		in_count = data.rows;
		covar_flags |= CV_COVAR_ROWS;
		mean_sz = Size(len, 1);
	}

	CV_Assert( retainedVariance > 0 && retainedVariance <= 1 );

	int count = std::min(len, in_count);

	// "scrambled" way to compute PCA (when cols(A)>rows(A)):
	// B = A'A; B*x=b*x; C = AA'; C*y=c*y -> AA'*y=c*y -> A'A*(A'*y)=c*(A'*y) -> c = b, x=A'*y
	if( len <= in_count )
		covar_flags |= CV_COVAR_NORMAL;

	int ctype = std::max(CV_32F, data.depth());
	mean.create( mean_sz, ctype );

	Mat covar( count, count, ctype );

	if( _mean.data )
	{
		CV_Assert( _mean.size() == mean_sz );
		_mean.convertTo(mean, ctype);
	}

	calcCovarMatrix( data, covar, mean, covar_flags, ctype );
	eigen( covar, eigenvalues, eigenvectors );

	if( !(covar_flags & CV_COVAR_NORMAL) )
	{
		// CV_PCA_DATA_AS_ROW: cols(A)>rows(A). x=A'*y -> x'=y'*A
		// CV_PCA_DATA_AS_COL: rows(A)>cols(A). x=A''*y -> x'=y'*A'
		Mat tmp_data, tmp_mean = repeat(mean, data.rows/mean.rows, data.cols/mean.cols);
		if( data.type() != ctype || tmp_mean.data == mean.data )
		{
			data.convertTo( tmp_data, ctype );
			subtract( tmp_data, tmp_mean, tmp_data );
		}
		else
		{
			subtract( data, tmp_mean, tmp_mean );
			tmp_data = tmp_mean;
		}

		Mat evects1(count, len, ctype);
		gemm( eigenvectors, tmp_data, 1, Mat(), 0, evects1,
			(flags & CV_PCA_DATA_AS_COL) ? CV_GEMM_B_T : 0);
		eigenvectors = evects1;

		// normalize all eigenvectors
		for( i = 0; i < eigenvectors.rows; i++ )
		{
			Mat vec = eigenvectors.row(i);
			normalize(vec, vec);
		}
	}

	// compute the cumulative energy content for each eigenvector
	int L;
	if (ctype == CV_32F)
		L = computeCumulativeEnergy<float>(eigenvalues, retainedVariance);
	else
		L = computeCumulativeEnergy<double>(eigenvalues, retainedVariance);

	// use clone() to physically copy the data and thus deallocate the original matrices
	eigenvalues = eigenvalues.rowRange(0,L).clone();
	eigenvectors = eigenvectors.rowRange(0,L).clone();
}

void ACP::project(InputArray _data, OutputArray result) const {
	Mat data = _data.getMat();
	CV_Assert( mean.data && eigenvectors.data && ((mean.rows == 1 && mean.cols == data.cols) || (mean.cols == 1 && mean.rows == data.rows)));
	Mat tmp_data, tmp_mean = repeat(mean, data.rows/mean.rows, data.cols/mean.cols);
	int ctype = mean.type();
	if( data.type() != ctype || tmp_mean.data == mean.data )
	{
		data.convertTo( tmp_data, ctype );
		subtract( tmp_data, tmp_mean, tmp_data );
	}
	else
	{
		subtract( data, tmp_mean, tmp_mean );
		tmp_data = tmp_mean;
	}
	if( mean.rows == 1 )
		gemm( tmp_data, eigenvectors, 1, Mat(), 0, result, GEMM_2_T );
	else
		gemm( eigenvectors, tmp_data, 1, Mat(), 0, result, 0 );
}

Mat ACP::project(InputArray data) const {
	Mat result;
	project(data, result);
	return result;
}

void ACP::backProject(InputArray _data, OutputArray result) const {
	Mat data = _data.getMat();
	CV_Assert( mean.data && eigenvectors.data && ((mean.rows == 1 && eigenvectors.rows == data.cols) || (mean.cols == 1 && eigenvectors.rows == data.rows)));

	Mat tmp_data, tmp_mean;
	data.convertTo(tmp_data, mean.type());
	if( mean.rows == 1 )
	{
		tmp_mean = repeat(mean, data.rows, 1);
		gemm( tmp_data, eigenvectors, 1, tmp_mean, 1, result, 0 );
	}
	else
	{
		tmp_mean = repeat(mean, 1, data.cols);
		gemm( eigenvectors, tmp_data, 1, tmp_mean, 1, result, GEMM_1_T );
	}
}

Mat ACP::backProject(InputArray data) const
{
	Mat result;
	backProject(data, result);
	return result;
}

