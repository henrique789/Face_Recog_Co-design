#include "autofaces.h"

//MAX BUFFER FOR DMA
#define MAXDMA 20

//BASE ADDRESS FOR CONTROL REGISTER
#define CRA 0x00000000

//BASE ADDRESS TO SDRAM
#define SDRAM 0x08000000

#define RWSIZE (32 / 8)

PCIE_BAR pcie_bars[] = { PCIE_BAR0, PCIE_BAR1 , PCIE_BAR2 , PCIE_BAR3 , PCIE_BAR4 , PCIE_BAR5 };

// Reads a sequence from a FileNode::SEQ with type _Tp into a result vector.
template<typename _Tp>
inline void readFileNodeList(const FileNode& fn, vector<_Tp>& result) {
	if (fn.type() == FileNode::SEQ) {
		for (FileNodeIterator it = fn.begin(); it != fn.end();) {
			_Tp item;
			it >> item;
			result.push_back(item);
		}
	}
}

// Writes the a list of given items to a cv::FileStorage.
template<typename _Tp>
inline void writeFileNodeList(FileStorage& fs, const string& name, const vector<_Tp>& items) {
	// typedefs
	typedef typename vector<_Tp>::const_iterator constVecIterator;
	// write the elements in item to fs
	fs << name << "[";
	for (constVecIterator it = items.begin(); it != items.end(); ++it) {
		fs << *it;
	}
	fs << "]";
}

static Mat asRowMatrix(InputArrayOfArrays src, int rtype, double alpha=1, double beta=0) {
	// make sure the input data is a vector of matrices or vector of vector
	if(src.kind() != _InputArray::STD_VECTOR_MAT && src.kind() != _InputArray::STD_VECTOR_VECTOR) {
		string error_message = "The data is expected as InputArray::STD_VECTOR_MAT (a std::vector<Mat>) or _InputArray::STD_VECTOR_VECTOR (a std::vector< vector<...> >).";
		CV_Error(CV_StsBadArg, error_message);
	}
	// number of samples
	size_t n = src.total();
	// return empty matrix if no matrices given
	if(n == 0)
		return Mat();
	// dimensionality of (reshaped) samples
	size_t d = src.getMat(0).total();
	// create data matrix
	Mat data((int)n, (int)d, rtype);

	// now copy data
	for(unsigned int i = 0; i < n; i++) {
		// make sure data can be reshaped, throw exception if not!
		if(src.getMat(i).total() != d) {
			string error_message = format("Wrong number of elements in matrix #%d! Expected %d was %d.", i, d, src.getMat(i).total());
			CV_Error(CV_StsBadArg, error_message);
		}
		// get a hold of the current row
		Mat xi = data.row(i);
		// make reshape happy by cloning for non-continuous matrices
		if(src.getMat(i).isContinuous()) {
			src.getMat(i).reshape(1, 1).convertTo(xi, rtype, alpha, beta);
		} else {
			src.getMat(i).clone().reshape(1, 1).convertTo(xi, rtype, alpha, beta);
		}
	}
	return data;
}

// Removes duplicate elements in a given vector.
template<typename _Tp>
inline vector<_Tp> remove_dups(const vector<_Tp>& src) {
	typedef typename set<_Tp>::const_iterator constSetIterator;
	typedef typename vector<_Tp>::const_iterator constVecIterator;
	set<_Tp> set_elems;
	for (constVecIterator it = src.begin(); it != src.end(); ++it)
		set_elems.insert(*it);
	vector<_Tp> elems;
	for (constSetIterator it = set_elems.begin(); it != set_elems.end(); ++it)
		elems.push_back(*it);
	return elems;
}

void Autofaces::train(InputArrayOfArrays _src, InputArray _local_labels) {
	if(_src.total() == 0) {
		string error_message = format("Empty training data was given. You'll need more than one sample to learn a model.");
		CV_Error(CV_StsBadArg, error_message);
	} else if(_local_labels.getMat().type() != CV_32SC1) {
		string error_message = format("Labels must be given as integer (CV_32SC1). Expected %d, but was %d.", CV_32SC1, _local_labels.type());
		CV_Error(CV_StsBadArg, error_message);
	}
	// make sure data has correct size
	if(_src.total() > 1) {
		for(int i = 1; i < static_cast<int>(_src.total()); i++) {
			if(_src.getMat(i-1).total() != _src.getMat(i).total()) {
				string error_message = format("In the Eigenfaces method all input samples (training images) must be of equal size! Expected %d pixels, but was %d pixels.", _src.getMat(i-1).total(), _src.getMat(i).total()); cout<<"i: "<<i<<endl;
				CV_Error(CV_StsUnsupportedFormat, error_message);
			}
		}
	}
	// get labels
	Mat labels = _local_labels.getMat();

	// observations in row------------------------------------------------------
	cout<<"\t\t\tstart asRowMatrix-------------------------"<<endl;
	monotonic_clock::time_point t1 = monotonic_clock::now();
	Mat data = asRowMatrix(_src, CV_32FC1);
	monotonic_clock::time_point t2 = monotonic_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\tasRowMatrix took " << time_span.count() << " seconds.\n";
	//--------------------------------------------------------------------------

	// number of samples
   int n = data.rows;
	// assert there are as much samples as labels
	if(static_cast<int>(labels.total()) != n) {
		string error_message = format("The number of samples (src) must equal the number of labels (labels)! len(src)=%d, len(labels)=%d.", n, labels.total());
		CV_Error(CV_StsBadArg, error_message);
	}
	// clear existing model data
	_labels.release();
	_projections.clear();
	// clip number of components to be valid
	//if((_num_components <= 0) || (_num_components > n))
		_num_components = 160;//n;

	//_num_components = 320;

	//perform the PCA-----------------------------------------------------------
	cout<<"\t\t\tstart PCA----------------------------------"<<endl;
	t1 = monotonic_clock::now();
	ACP acp(data, Mat(), CV_PCA_DATA_AS_ROW, _num_components);
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\tPCA took " << time_span.count() << " seconds.\n";
	//--------------------------------------------------------------------------

	//copy the PCA results------------------------------------------------------
	_mean = acp.mean.reshape(1,1); //store the mean vector
	_eigenvalues = acp.eigenvalues.clone(); //eigenvalues by row
	transpose(acp.eigenvectors, _eigenvectors); //eigenvectors by column
	Mat egvec = _eigenvectors.clone();
	// store labels for prediction
	_labels = labels.clone();
	//--------------------------------------------------------------------------

	/*cout<<"data type: "<<data.type()<<endl;
	cout<<"mean type: "<<_mean.type()<<endl;
	cout<<"eigen type: "<<egvec.type()<<endl;
	*/
	// save projections----------------------------------------------------------
	cout<<"\t\t\tstart projections----------------------------------"<<endl;
	/*Mat data2 = Mat(data.rows, data.cols, CV_32FC1, Scalar(4));
	Mat mean2 = Mat(_mean.rows, _mean.cols, CV_32FC1, Scalar(1));
	Mat egvec2 = Mat(egvec.rows, egvec.cols, CV_32FC1, Scalar(3));
	*/
	t1 = monotonic_clock::now();
	/*for(int sampleIdx = 0; sampleIdx < data.rows; sampleIdx++) {
		Mat p = subspaceProject(egvec2, mean2, data2.row(sampleIdx));
		_projections.push_back(p);
		u_p = p.data;
	}*/

	FILE *f = fopen("pSW2.txt", "w");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	FILE *f3 = fopen("pSW3.txt", "w");
	if (f3 == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	for(int sampleIdx = 0; sampleIdx < data.rows; sampleIdx++) {
		Mat p = subspaceProject(_eigenvectors, _mean, data.row(sampleIdx));

		uchar *u_p = p.data;

		for (int j = 0; j < _num_components; j++){
			fprintf(f, "%4.4f ", p.at<float>(0,j));
		}
		fprintf(f, "\n");

		for (int j = 0; j < _num_components; j++){
			fprintf(f3, "%lX ", *(uint*)&p.at<float>(0,j));
		}
		fprintf(f3, "\n");

		_projections.push_back(p);
	}
	fclose(f);
	fclose(f3);

	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\tsubspaceProject took " << time_span.count() << " seconds.\n";
}

void Autofaces::train(PCIE_HANDLE hPCIe, InputArrayOfArrays _src, InputArray _local_labels) {
	if(_src.total() == 0) {
		string error_message = format("Empty training data was given. You'll need more than one sample to learn a model.");
		CV_Error(CV_StsBadArg, error_message);
	} else if(_local_labels.getMat().type() != CV_32SC1) {
		string error_message = format("Labels must be given as integer (CV_32SC1). Expected %d, but was %d.", CV_32SC1, _local_labels.type());
		CV_Error(CV_StsBadArg, error_message);
	}

	// make sure data has correct size
	if(_src.total() > 1) {
		for(int i = 1; i < static_cast<int>(_src.total()); i++) {
			if(_src.getMat(i-1).total() != _src.getMat(i).total()) {
				string error_message = format("In the Eigenfaces method all input samples (training images) must be of equal size! Expected %d pixels, but was %d pixels.", _src.getMat(i-1).total(), _src.getMat(i).total()); cout<<"i: "<<i<<endl;
				CV_Error(CV_StsUnsupportedFormat, error_message);
			}
		}
	}

	// get labels
	Mat labels = _local_labels.getMat();

	// observations in row------------------------------------------------------
	cout<<"\t\t\tstart asRowMatrix-------------------------"<<endl;
	monotonic_clock::time_point t1 = monotonic_clock::now();
	Mat data = asRowMatrix(_src, CV_32FC1);
	monotonic_clock::time_point t2 = monotonic_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\tasRowMatrix took " << time_span.count() << " seconds.\n";
	//--------------------------------------------------------------------------

	// number of samples
   int n = data.rows;
	// assert there are as much samples as labels
	if(static_cast<int>(labels.total()) != n) {
		string error_message = format("The number of samples (src) must equal the number of labels (labels)! len(src)=%d, len(labels)=%d.", n, labels.total());
		CV_Error(CV_StsBadArg, error_message);
	}
	// clear existing model data
	_labels.release();
	_projections.clear();
	// clip number of components to be valid
	//if((_num_components <= 0) || (_num_components > n))
		_num_components = 160;//n;

	//perform the PCA-----------------------------------------------------------
	cout<<"\t\t\tstart PCA----------------------------------"<<endl;
	t1 = monotonic_clock::now();
	ACP acp(data, Mat(), CV_PCA_DATA_AS_ROW, _num_components);
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\tPCA took " << time_span.count() << " seconds.\n";
	//--------------------------------------------------------------------------

	//copy the PCA results------------------------------------------------------
	_mean = acp.mean.reshape(1,1); //store the mean vector
	_eigenvalues = acp.eigenvalues.clone(); //eigenvalues by row
	transpose(acp.eigenvectors, _eigenvectors); //eigenvectors by column
	Mat egvec = acp.eigenvectors.clone();
	// store labels for prediction
	_labels = labels.clone();
	//--------------------------------------------------------------------------
	
	// save projections----------------------------------------------------------
	cout<<"\t\t\tstart projections----------------------------------"<<endl;
	/*Mat data2 = Mat(data.rows, data.cols, CV_32FC1, Scalar(4));
	Mat mean2 = Mat(_mean.rows, _mean.cols, CV_32FC1, Scalar(1));
	Mat egvec2 = Mat(egvec.rows, egvec.cols, CV_32FC1, Scalar(3));
	*/
	t1 = monotonic_clock::now();

	//FPGAsubspaceProject(hPCIe, data2, mean2, egvec2);
	FPGAsubspaceProject(hPCIe, data, _mean, egvec);

	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\tsubspaceProject took " << time_span.count() << " seconds.\n";
}

BOOL Autofaces::ReadProjsDMA(PCIE_HANDLE hPCIe, Mat src){
	DWORD projs_addr = 0x0960D100;

	FILE *f = fopen("pHW2.txt", "w");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	FILE *f3 = fopen("pHW3.txt", "w");
	if (f3 == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	float *proj;
	proj = (float*)malloc(_num_components*sizeof(float));

	for (int i = 0; i < src.rows; i++){
		BOOL bPass = PCIE_DmaRead(hPCIe, projs_addr, proj, _num_components*sizeof(float));
		if(!bPass){
			printf("ERROR: unsuccessful Eigenface reading.\n");
			return FALSE;
		}
		
		for (int j = 0; j < _num_components; j++){
			fprintf(f, "%4.4f ", proj[j]);
		}
		fprintf(f, "\n");

		for (int j = 0; j < _num_components; j++){
			fprintf(f3, "%lX ", *(uint*)&proj[j]);
		}
		fprintf(f3, "\n");


		Mat dst = Mat(1, _num_components, CV_32FC1, proj);
		_projections.push_back(dst.clone());
		projs_addr = projs_addr + 640;
	}


	fclose(f);
	fclose(f3);
}

BOOL Autofaces::ReadProjsDMA2(PCIE_HANDLE hPCIe, Mat src){
	DWORD projs_addr = 0x0960D100;

	FILE *f = fopen("pHW3.txt", "w");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	float *proj;
	proj = (float*)malloc(_num_components*sizeof(float));

	for (int i = 0; i < src.rows; i++){
		BOOL bPass = PCIE_DmaRead(hPCIe, projs_addr, proj, _num_components*sizeof(float));
		if(!bPass){
			printf("ERROR: unsuccessful Eigenface reading.\n");
			return FALSE;
		}
		
		for (int j = 0; j < _num_components; j++)
		{
			fprintf(f, "%lX ", *(uint*)&proj[j]);
		}
		fprintf(f, "\n");

		projs_addr = projs_addr + 640;
	}
	//fclose(f);
}

BOOL WriteEigenfacesDMA(PCIE_HANDLE hPCIe, Mat src){
	uchar *u_egvec = src.data;
	DWORD egvec_addr = 0x08FC3100;

	int awsd = src.rows*src.cols;
	BOOL egvecPass = PCIE_DmaWrite(hPCIe, egvec_addr, u_egvec, awsd*sizeof(float));

	if (!egvecPass){
		printf("ERROR: unsuccessful eigenfaces writing.\n");
		return FALSE;
	}else{
		printf("\t\t\t\tEigenfaces written.\n");
	}

	return TRUE;
}

BOOL WriteFacesDMA(PCIE_HANDLE hPCIe, Mat src){
	uchar *u_faces = src.data;
	DWORD faces_addr = 0x0800A100;

	int awsd = src.rows*src.cols;
	BOOL facesPass = PCIE_DmaWrite(hPCIe, faces_addr, u_faces, awsd*sizeof(float));

	if (!facesPass){
		printf("ERROR: unsuccessful faces writing.\n");
		return FALSE;
	}else{
		printf("\t\t\t\tFaces written.\n");
	}

	return TRUE;
}

BOOL WriteMeanDMA(PCIE_HANDLE hPCIe, Mat src){
	uchar *u_mean = src.data;
	DWORD mean_addr = 0x08000000;

	BOOL meanPass = PCIE_DmaWrite(hPCIe, mean_addr, u_mean, src.cols*sizeof(float));

	if (!meanPass) {
		printf("ERROR: unsuccessful mean writing.\n");
		return FALSE;
	} else {
		printf("\t\t\t\tMean written.\n");
	}

	return TRUE;
}

BOOL checkImageDone(PCIE_HANDLE hPCIe){
	BYTE b;
	DWORD addr = 0x00000000;
	BOOL bPass = PCIE_Read8( hPCIe, pcie_bars[0], addr, &b);
	BYTE check = 0x12;
	if(bPass) {
		if(b == check) {
			printf("\t\t\t\tImage done\n");
			return TRUE;
		} else {
			return FALSE;
		}
	}
	return FALSE;
}

BOOL WriteStartByte(PCIE_HANDLE hPCIe){
	DWORD addr = 0x00000000;
	BYTE start = 0x53;

	BOOL bPass = PCIE_Write32( hPCIe, pcie_bars[0], addr, start);
	if(!bPass){
		printf("ERROR: unsuccessful start byte writing.\n");
		return FALSE;
	}else
		printf("\t\t\t\tStart byte written.\n");
	return TRUE;
}

BOOL check_pixel_iter(PCIE_HANDLE hPCIe){
	DWORD b;
	DWORD addr = 0x00000004;

	FILE *f = fopen("pixel_iter.txt", "a");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	BOOL bPass = PCIE_Read32( hPCIe, pcie_bars[0], addr, &b);
	if(bPass) {
		fprintf(f, "%lX\n", b);
		fclose(f);
		return TRUE;
	}
	fclose(f);
	return FALSE;
}

BOOL check_weight_iter(PCIE_HANDLE hPCIe){
	DWORD b;
	DWORD addr = 0x00000008;

	FILE *f = fopen("weight_iter.txt", "a");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	BOOL bPass = PCIE_Read32( hPCIe, pcie_bars[0], addr, &b);
	if(bPass) {
		fprintf(f, "%lX\n", b);
		fclose(f);
		return TRUE;
	}
	fclose(f);
	return FALSE;
}

BOOL check_sample_iter(PCIE_HANDLE hPCIe){
	DWORD b;
	DWORD addr = 0x0000000C;

	FILE *f = fopen("sample_iter.txt", "a");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	BOOL bPass = PCIE_Read32( hPCIe, pcie_bars[0], addr, &b);
	if(bPass) {
		fprintf(f, "%lX\n", b);
		fclose(f);
		return TRUE;
	}
	fclose(f);
	return FALSE;
}

BOOL check_pixel_b(PCIE_HANDLE hPCIe){
	DWORD b;
	DWORD addr = 0x00000010;

	FILE *f = fopen("pixel_b.txt", "a");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	BOOL bPass = PCIE_Read32( hPCIe, pcie_bars[0], addr, &b);
	if(bPass) {
		fprintf(f, "%lX\n", b);
		fclose(f);
		return TRUE;
	}
	fclose(f);
	return FALSE;
}

BOOL check_pixel_e(PCIE_HANDLE hPCIe){
	DWORD b;
	DWORD addr = 0x00000014;

	FILE *f = fopen("pixel_e.txt", "a");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	BOOL bPass = PCIE_Read32( hPCIe, pcie_bars[0], addr, &b);
	if(bPass) {
		fprintf(f, "%lX\n", b);
		fclose(f);
		return TRUE;
	}
	fclose(f);
	return FALSE;
}

BOOL check_r_address(PCIE_HANDLE hPCIe){
	DWORD b;
	DWORD addr = 0x00000018;

	FILE *f = fopen("r_address.txt", "a");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	BOOL bPass = PCIE_Read32( hPCIe, pcie_bars[0], addr, &b);
	if(bPass) {
		fprintf(f, "%lX\n", b);
		fclose(f);
		return TRUE;
	}
	fclose(f);
	return FALSE;
}

BOOL check_w_address(PCIE_HANDLE hPCIe){
	DWORD b;
	DWORD addr = 0x0000001C;

	FILE *f = fopen("w_address.txt", "a");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	BOOL bPass = PCIE_Read32( hPCIe, pcie_bars[0], addr, &b);
	if(bPass) {
		fprintf(f, "%lX\n", b);
		fclose(f);
		return TRUE;
	}
	fclose(f);
	return FALSE;
}

BOOL clearSDRAM(PCIE_HANDLE hPCIe){
	DWORD projs_addr = 0x0960D100;

	float *proj;
	proj = (float*)calloc(400*160,sizeof(float));

	BOOL bPass = PCIE_DmaWrite(hPCIe, projs_addr, proj, 400*160*sizeof(float));
	if(!bPass){
		printf("ERROR: unsuccessful Eigenface reading.\n");
		return FALSE;
	}

	return TRUE;
}

BOOL checkVals(PCIE_HANDLE hPCIe){
	DWORD m_addr = 0x08000000;

	FILE *f = fopen("meanR.txt", "w");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	float *mean;
	mean = (float*)malloc(10304*sizeof(float));
	BOOL bPass = PCIE_DmaRead(hPCIe, m_addr, mean, 10304*sizeof(float));
	if(!bPass){
		printf("ERROR: unsuccessful mean reading.\n");
		return FALSE;
	}
	for (int j = 0; j < 10304; j++){
		fprintf(f, "%lX ", *(uint*)&mean[j]);
	}
	fclose(f);
//-------------------------------------------------------------------------------------------
	DWORD f_addr = 0x0800A100;

	FILE *f2 = fopen("facesR.txt", "w");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	float *face;
	face = (float*)malloc(10304*sizeof(float));
	for(int i = 0; i < 400; i++){
		BOOL bPass = PCIE_DmaRead(hPCIe, f_addr, face, 10304*sizeof(float));
		if(!bPass){
			printf("ERROR: unsuccessful face reading.\n");
			return FALSE;
		}
		for (int j = 0; j < 10304; j++){
			fprintf(f2, "%lX ", *(uint*)&face[j]);
		}
		fprintf(f2, "\n");
		f_addr += 10304*4;
	}
	fclose(f2);
//-------------------------------------------------------------------------------------------
	DWORD e_addr = 0x08FC3100;

	FILE *f3 = fopen("eigenR.txt", "w");
	if (f3 == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	float *eigen;
	eigen = (float*)malloc(10304*sizeof(float));
	for(int i = 0; i < 160; i++){
		BOOL bPass = PCIE_DmaRead(hPCIe, e_addr, eigen, 10304*sizeof(float));
		if(!bPass){
			printf("ERROR: unsuccessful eigen reading.\n");
			return FALSE;
		}
		for (int j = 0; j < 10304; j++){
			fprintf(f3, "%lX ", *(uint*)&eigen[j]);
		}
		fprintf(f3, "\n");
		e_addr += 10304*4;
	}
	fclose(f3);

	return TRUE;
}

void Autofaces::FPGAsubspaceProject(PCIE_HANDLE hPCIe, Mat src, Mat mean, Mat egvec){
	monotonic_clock::time_point t1;
	monotonic_clock::time_point t2;
	duration<double> time_span;

	t1 = monotonic_clock::now();
	if(!WriteMeanDMA(hPCIe, mean))return;
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\t\tWriteMeanDMA took " << time_span.count() << " seconds.\n";

	t1 = monotonic_clock::now();
	if(!WriteFacesDMA(hPCIe, src))return;
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\t\tWriteFacesDMA took " << time_span.count() << " seconds.\n";

	t1 = monotonic_clock::now();
	if(!WriteEigenfacesDMA(hPCIe, egvec))return;
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\t\tWriteEigenfacesDMA took " << time_span.count() << " seconds.\n";

	//checkVals(hPCIe);

	t1 = monotonic_clock::now();
	if(!WriteStartByte(hPCIe))return;
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\t\tWriteStartByte took " << time_span.count() << " seconds.\n";

	
	while(!checkImageDone(hPCIe));

	t1 = monotonic_clock::now();
	ReadProjsDMA(hPCIe, src);
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\t\t\t\tReadProjsDMA took " << time_span.count() << " seconds.\n";

	//if(!checkVals(hPCIe))return;
}

void Autofaces::predict(InputArray _src, int &minClass, double &minDist) const {
	// get data
	Mat src = _src.getMat();
	// make sure the user is passing correct data
	if(_projections.empty()) {
		// throw error if no data (or simply return -1?)
		string error_message = "This Eigenfaces model is not computed yet. Did you call Eigenfaces::train?";
		CV_Error(CV_StsError, error_message);
	} else if(_eigenvectors.rows != static_cast<int>(src.total())) {
		// check data alignment just for clearer exception messages
		string error_message = format("Wrong input image size. Reason: Training and Test images must be of equal size! Expected an image with %d elements, but got %d.", _eigenvectors.rows, src.total());
		CV_Error(CV_StsBadArg, error_message);
	}

	//Mat data2 = Mat(src.rows, src.cols, CV_32FC1, Scalar(4));
	//Mat mean2 = Mat(_mean.rows, _mean.cols, CV_32FC1, Scalar(1));
	//Mat egvec2 = Mat(_eigenvectors.rows, _eigenvectors.cols, CV_32FC1, Scalar(3));

	// project into PCA subspace
	//Mat q = subspaceProject(egvec2, mean2, data2.reshape(1,1));
	Mat q = subspaceProject(_eigenvectors, _mean, src.reshape(1,1));
	//cout<<"proj type: "<<q.type()<<endl;
	minDist = DBL_MAX;
	minClass = -1;
	for(size_t sampleIdx = 0; sampleIdx < _projections.size(); sampleIdx++) {
		double dist = norm(_projections[sampleIdx], q, NORM_L2);
		if((dist < minDist) && (dist < _threshold)) {
			minDist = dist;
			minClass = _labels.at<int>((int)sampleIdx);
		}
	}
	cout<<"MinDist: "<<minDist<<endl;
}

int Autofaces::predict(InputArray _src) const {
	int label;
	double dummy;
	predict(_src, label, dummy);
	return label;
}

void Autofaces::load(const FileStorage& fs) {
	//read matrices
	fs["num_components"] >> _num_components;
	fs["mean"] >> _mean;
	fs["eigenvalues"] >> _eigenvalues;
	fs["eigenvectors"] >> _eigenvectors;
	// read sequences
	readFileNodeList(fs["projections"], _projections);
	fs["labels"] >> _labels;
}

void Autofaces::save(FileStorage& fs) const {
	// write matrices
	fs << "num_components" << _num_components;
	fs << "mean" << _mean;
	fs << "eigenvalues" << _eigenvalues;
	fs << "eigenvectors" << _eigenvectors;
	// write sequences
	writeFileNodeList(fs, "projections", _projections);
	fs << "labels" << _labels;
}

Mat Autofaces::getMean(){
	return _mean.clone();
}

Mat Autofaces::getEigenVectors(){
	return _eigenvectors.clone();
}

Mat Autofaces::getEigenValues(){
	return _eigenvalues.clone();
}

Mat Autofaces::getProjections(){
	Mat dst;
	vconcat(_projections, dst);
	return dst;
}