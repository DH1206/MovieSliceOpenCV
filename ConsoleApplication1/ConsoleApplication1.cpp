
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>

using namespace cv;	//OpenCV Ver4.1

struct COORD{
	double x_centre;
	double y_centre;
	int width;
	int height;
};

int main(void) {
	cv::Mat img;
	cv::Mat gray_img;
	cv::Mat bin_img;
	cv::Mat LabelImg;
	cv::Mat stats;
	cv::Mat centroids;
	cv::VideoCapture cap;
	int frame_no;

	std::string VidName = "./image/test1.MP4";	//動画ファイル名の指定
	std::string CsvName = "./image/test1.CSV";	//出力CSV名の指定　（面倒なので、直接指定・・・）

	cap = cv::VideoCapture(VidName); //動画読み込み
	if (!cap.isOpened()) {			// 読み込みチェック
		std::cout << "File can not open!!\n";
		return -1;
	}

	//読みこんだ動画のプロパティ読み込み
	int v_w = cap.get(CAP_PROP_FRAME_WIDTH); //横の大きさ
	int v_h = cap.get(CAP_PROP_FRAME_HEIGHT); //縦の大きさ
	int max_frame = cap.get(CAP_PROP_FRAME_COUNT); //フレーム数
	int fps = cap.get(CAP_PROP_FPS); //フレームレート
	std::cout << "Width: " << v_w << "\n";
	std::cout << "Height: " << v_h << "\n";
	std::cout << "Frames: " << max_frame << "\n";
	std::cout << "FPS: " << fps << "\n";

	//結果の配列用意
	std::vector<COORD> Output(max_frame);

	//各フレーム毎の処理
	for (frame_no = 0; frame_no < max_frame; frame_no++) {
		//フレーム切り出し
		std::cout << "\r" << "Processing Flame No: #" << frame_no << std::string(30, ' ');
//		cap.set(CAP_PROP_POS_FRAMES,i);	//フレーム位置指定　（動画の途中から始める時の指定用）
		cap >> img;	//最初のフレームから全部処理するなら、これだけで良い

		//RGBを変換
		cvtColor(img, gray_img, COLOR_BGR2GRAY); //グレースケール(CV_8UC1)に変換 
		threshold(gray_img, bin_img, 30, 255, THRESH_BINARY); //閾値(適当)で2値画像に変換	★要調整 閾値30で、Matlab側の処理と大体一致する。
//		threshold(gray_img, bin_img, 0, 255, THRESH_BINARY | THRESH_OTSU); //閾値を大津の方式で自動で設定

//		imshow("Bin Image", bin_img);	//debug用　Windowに表示しながら処理
//		waitKey(1);						// debug用　上記の表示のために1ms待つ

		//画像解析でラベリング
		int nLab = cv::connectedComponentsWithStats(bin_img, LabelImg, stats, centroids, 8);	//８方向連結の領域を解析 戻り値はラベル数 (背景が0番目）

		if (nLab != 2) {				//点の領域が１つのみという事を想定　（背景＋点像でラベル数２）
			std::cout << "Detect Error!\n";
		}

		//重心の出力
		cv::Mat Dst(bin_img.size(), CV_8UC3);
		Dst = bin_img;									//入力画像のコピー

		int* param_stats = stats.ptr<int>(1);			//点像の大きさ取得 点像の領域のラベル番号は１
		double* param_cent = centroids.ptr<double>(1);	//点像の中心座標取得
		int height = param_stats[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
		int width = param_stats[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];
		
		//結果の保存
		Output[frame_no].x_centre = param_cent[0];
		Output[frame_no].y_centre = param_cent[1];
		Output[frame_no].width = width;
		Output[frame_no].height = height;

		std::cout << "X: " << Output[frame_no].x_centre << "Y: " << Output[frame_no].y_centre << "W: " << Output[frame_no].width << "H: " << Output[frame_no].height << "\n";

		//int x = static_cast<int>(param_cent[0]);	//debug用
		//int y = static_cast<int>(param_cent[1]);	//debug用
		//cv::circle(Dst, cv::Point(x, y), 1, cv::Scalar(0, 0, 255), 1);	//重心座標に点を打って上書き
		//imshow("Centroid Image", Dst);					//debug用　Windowに表示しながら処理
		//waitKey(1);										//debug用　上記の表示のために1ms待つ
	};

	//csvへ結果の書き出し
	std::ofstream ofs(CsvName);	//閉じるのは勝手にやってくれるはず
	for(frame_no = 0; frame_no < max_frame; frame_no++) {
		ofs << Output[frame_no].x_centre << ",";
		ofs << Output[frame_no].y_centre << ",";
		ofs << Output[frame_no].width << ","; 
		ofs << Output[frame_no].height << std::endl;
	}

	std::cout << std::endl;
	cap.release();	//キャプチャリリース（一応）


	return 0;
}
