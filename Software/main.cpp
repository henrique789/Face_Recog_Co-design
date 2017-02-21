#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <memory.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "autofaces.h"

#include "PCIE.h"

using namespace cv;
using namespace std;
using namespace std::chrono;

string path = "/home/Altera/Documentos/att_faces/";
string output_folder = string(path+"output01");
string fn_csv = string(path+"names.csv");
vector<Mat> images;
vector<int> labels;

String face_cascade_name = "/home/Altera/Documentos/opencv-2.4.9/data/haarcascades/haarcascade_frontalface_alt.xml";
CascadeClassifier face_cascade;
string window_name = "Capture - Face detection";

Mat eigenvalues;
Mat W;
Mat mean_;

int im_width;
int im_height;

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {

	ifstream file(filename.c_str(), ifstream::in);
	if (!file) {
		string error_message = "No valid input file was given, please check the given filename.";
		CV_Error(CV_StsBadArg, error_message);
	}
	string line, path, classlabel;
	while (getline(file, line)) {
		stringstream liness(line);
		getline(liness, path, separator);
		getline(liness, classlabel);
		if(!path.empty() && !classlabel.empty()) {
			images.push_back(imread(path, 0));
			labels.push_back(atoi(classlabel.c_str()));
		}
	}
}

static Mat norm_0_255(InputArray _src) {
	Mat src = _src.getMat();
	// Create and return normalized image:
	Mat dst;
	switch(src.channels()) {
	case 1:
		normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
		break;
	case 3:
		normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
		break;
	default:
		src.copyTo(dst);
		break;
	}
	return dst;
}

static void save_ef(InputArray evec_src, InputArray eval_src){
	Mat evec = evec_src.getMat();
	Mat eval = eval_src.getMat();

	int height = images[0].rows;
	for (int i = 0; i < min(240, evec.cols); i++) {
		string msg = format("Eigenvalue #%d = %.5f", i, eval.at<double>(i));
		cout << msg << endl;
		// get eigenvector #i
		Mat ev = evec.col(i).clone();
		// Reshape to original size & normalize to [0...255] for imshow.
		Mat grayscale = norm_0_255(ev.reshape(1, height));
		// Show the image & apply a Jet colormap for better sensing.
		//Mat cgrayscale;
		//applyColorMap(grayscale, cgrayscale, COLORMAP_JET);

		// Display or save:
		//imshow(format("eigenface_%d", i), cgrayscale);
		imwrite(format("%s/eigenface_%d.png", output_folder.c_str(), i), grayscale/*norm_0_255(cgrayscale)*/);
	}
}

static void save_recon(InputArray mean_src, InputArray sample_src){
	Mat testRecons = sample_src.getMat().clone();
	Mat mean_0 = mean_src.getMat();
	for(int num_components = 1; num_components < 240; num_components+=8) {
		// slice the eigenvectors from the model
		Mat evs = Mat(W, Range::all(), Range(0, num_components));
		Mat projection = subspaceProject(evs, mean_0, testRecons.reshape(1,1));
		Mat reconstruction = subspaceReconstruct(evs, mean_0, projection);
		// Normalize the result:
		reconstruction = norm_0_255(reconstruction.reshape(1, testRecons.rows));

		// Display or save:
		//imshow(format("eigenface_reconstruction_%d", num_components), reconstruction);
		imwrite(format("%s/eigenface_reconstruction_%d.png", output_folder.c_str(), num_components), reconstruction);
	}
}

int main()
{
	void *lib_handle;
	PCIE_HANDLE hPCIe;

	lib_handle = PCIE_Load();
	if (!lib_handle)
	{
		printf("PCIE_Load failed\n");
		return 0;
	}
	hPCIe = PCIE_Open(0,0,0);

	if (!hPCIe)
	{
		printf("PCIE_Open failed\n");
		return 0;
	}
	
	try {
		read_csv(fn_csv, images, labels);
	} catch (cv::Exception& e) {
		cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
		// nothing more we can do
		exit(1);
	}
	if(images.size() <= 1) {
		string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
		CV_Error(CV_StsError, error_message);
	}

	monotonic_clock::time_point t1;
	monotonic_clock::time_point t2;
	duration<double> time_span;
	string result_message;
	int predicted;

	Mat testSample = images[10];
	int testLabel = labels[10];
	im_width = images[0].cols;
	im_height = images[0].rows;
	cout<<"w: "<<im_width<<endl;
	cout<<"h: "<<im_height<<endl;
	cout<<"type: "<<images[0].type()<<endl;
	cout<<"channels: "<<images[0].channels()<<endl;

	Autofaces *model = new Autofaces();

	cout<<"Co-Design---------------------------------------"<<endl;

	cout<<"\tstart training----------------------------------"<<endl;
	t1 = monotonic_clock::now();
	model->train(hPCIe, images, labels);
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\ttraining took " << time_span.count() << " seconds.\n";

	Mat pHW = model->getProjections();
	//Mat tstS = Mat(testSample.rows, testSample.cols, CV_8UC1, Scalar(4));
	
	cout<<"\tstart prediction----------------------------------"<<endl;
	t1 = monotonic_clock::now();
	predicted = model->predict(testSample);
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\tprediction took " << time_span.count() << " seconds.\n";
	
	//W = model->getEigenVectors();
	//mean_ = model->getMean();
	//eigenvalues = model->getEigenValues();

	//save_recon(mean_, testSample);
	//save_ef(W, eigenvalues);

	result_message = format("\tCoproj: Predicted class = %d / Actual class = %d.\n\n", predicted, testLabel);

	cout << result_message << endl;

	cout<<"pHW rows: "<<pHW.rows<<endl;
	cout<<"pHW cols: "<<pHW.cols<<endl;
	cout<<"pHW type: "<<pHW.type()<<endl;
	cout<<"pHW channels: "<<pHW.channels()<<endl;

	FILE *f = fopen("pHW.txt", "w");
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}
	for (int i = 0; i < pHW.rows; i++){
		float* pHWi = pHW.ptr<float>(i);
		for (int j = 0; j < pHW.cols; j++){
			fprintf(f, "%lX ", *(uint*)&pHWi[j]);
		}
		fprintf(f, "\n");
	}
	fclose(f);
	

	cout<<"Pure-SW-----------------------------------------"<<endl;
	predicted = 0;
	Autofaces *model2 = new Autofaces();

	cout<<"\tstart training----------------------------------"<<endl;
	t1 = monotonic_clock::now();
	model2->train(images, labels);
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\ttraining took " << time_span.count() << " seconds.\n";

	Mat pSW = model2->getProjections();

	cout<<"\tstart prediction----------------------------------"<<endl;
	t1 = monotonic_clock::now();
	predicted = model2->predict(testSample);
	t2 = monotonic_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	cout << "\tprediction took " << time_span.count() << " seconds.\n";

	result_message = format("\tFullSW: Predicted class = %d / Actual class = %d.", predicted, testLabel);

	cout << result_message << endl;
	
	cout<<"pSW rows: "<<pSW.rows<<endl;
	cout<<"pSW cols: "<<pSW.cols<<endl;
	cout<<"pSW type: "<<pSW.type()<<endl;
	cout<<"pSW channels: "<<pSW.channels()<<endl;

	FILE *f2 = fopen("pSW.txt", "w");
	if (f2 == NULL){
		printf("Error opening file!\n");
		exit(1);
	}
	for (int i = 0; i < pSW.rows; i++){
		float* pSWi = pSW.ptr<float>(i);
		for (int j = 0; j < pSW.cols; j++){
			fprintf(f2, "%lX ", *(uint*)&pSWi[j]);
		}
		fprintf(f2, "\n");
	}
	fclose(f2);



	/*Mat m1 = Mat(1, 4, CV_8UC1, Scalar(6));
	Mat m2 = Mat(1, 4, CV_8UC1, Scalar(7));

	Mat m3;

	hconcat(m1,m2,m3);
	cout<<"rows: "<<m1.rows<<endl;
	cout<<"cols: "<<m1.cols<<endl;
	cout<<"type: "<<m1.type()<<endl;
	cout<<"channels: "<<m1.channels()<<endl;

	cout<<"rows: "<<m3.rows<<endl;
	cout<<"cols: "<<m3.cols<<endl;
	cout<<"type: "<<m3.type()<<endl;
	cout<<"channels: "<<m3.channels()<<endl;

	uchar *uvec = m3.data;

	printf("%d\n%d\n",uvec[0],uvec[4]);*/
	//cout<<uvec[0]<<"\n"<<uvec[4]<<endl;

	return 0;
}