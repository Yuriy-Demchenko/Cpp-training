#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <chrono> 
#include <thread>
#include <mutex>

using namespace cv;
using namespace std;

struct Img {
    Mat inImg;
    Mat outImg;
};

std::mutex mtx;

void processImage(Mat &inImg, Mat &outImg, int h, int w);
void processImageTh(Mat &inImg, Mat &outImg, int h, int w);

int main( int argc, char** argv )
{
    Mat image;
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95);

    if( argc != 4)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    Mat outImage;
    
    Mat outputTh(atoi(argv[3]), atoi(argv[2]), CV_8UC3);
    Mat output(atoi(argv[3]), atoi(argv[2]), CV_8UC3);

    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    auto tStart = chrono::steady_clock::now();
    processImageTh(image, outputTh, atoi(argv[2]), atoi(argv[3]));
    auto tEnd = chrono::steady_clock::now();
    cout << "Running time with threads = " << chrono::duration_cast<chrono::milliseconds>(tEnd - tStart).count() << " ms" << endl;

    tStart = chrono::steady_clock::now();
    processImage(image, output, atoi(argv[2]), atoi(argv[3]));
    tEnd = chrono::steady_clock::now();
    cout << "Running time without threads = " << chrono::duration_cast<chrono::milliseconds>(tEnd - tStart).count() << " ms" << endl;

    imwrite("outputTh.jpg", outputTh, compression_params);
    imwrite("output.jpg", output, compression_params);
    return 0;
}

//void myResize(Img img, int i, int j, int w, int h, int wx, int hx, int wxx, int hxx)
void myResize(Mat &inImg, Mat &outImg, int i, int j, int w, int h, int wx, int hx, int wxx, int hxx)
{
    //Mat block = img.inImg(Rect(j*w, i*h, w, h));
    Mat block = inImg(Rect(j*w, i*h, w, h));
    resize(block, block, Size(), 1.0/wx, 1.0/hx, CV_INTER_LINEAR);
    //block(Rect(0, 0, wxx, hxx)).copyTo(img.outImg(Rect(j*wxx,i*hxx,wxx, hxx)));
    mtx.lock();
    block(Rect(0, 0, wxx, hxx)).copyTo(outImg(Rect(j*wxx,i*hxx,wxx, hxx)));
    mtx.unlock();
}

void myResize2(Mat &inImg, Mat &outImg, int i, int j, int w, int h, int wx, int hx, int wxx, int hxx)
{
    //Mat block = img.inImg(Rect(j*w, i*h, w, h));
    Mat block = inImg(Rect(j*w, i*h, w, h));
    resize(block, block, Size(), 1.0/wx, 1.0/hx, CV_INTER_LINEAR);
    //block(Rect(0, 0, wxx, hxx)).copyTo(img.outImg(Rect(j*wxx,i*hxx,wxx, hxx)));
    //mtx.lock();
    block(Rect(0, 0, wxx, hxx)).copyTo(outImg(Rect(j*wxx,i*hxx,wxx, hxx)));
    //mtx.unlock();
}

void bar(int &i)
{
    i = 5;
}

void processImageTh(Mat &inImg, Mat &outImg, int w, int h)
{
    int height = inImg.size().height;
    int width = inImg.size().width;
    int rc, k;
    Img myImg;
    myImg.inImg = inImg;

    cout << "W: " << width << endl;
    cout << "H: " << height << endl;
   
    int hx = height/h;
    int wx = width/w;

    std::thread t[hx*wx];

    cout << hx << endl;
    cout << wx << endl;

    int wxx = w/wx, hxx = h/hx;

    k = 0;
    for (int i = 0; i < hx; i++) {
        for (int j = 0; j < wx; j++) {
            //myResize(inImg, outImg, i, j, w, h, wx, hx, wxx, hxx);
            t[k] = std::thread(myResize, std::ref(inImg), std::ref(outImg), i, j, w, h, wx, hx, wxx, hxx);   
            k++;
        }
    }
    //int i = 0, j = 0;


    //std::thread second(bar, std::ref(k)); 

    for (int i = 0; i < k; i++)
       t[i].join();
    //outImg = myImg.outImg;
    //second.join();
    //cout << k << endl;
}

void processImage(Mat &inImg, Mat &outImg, int w, int h)
{
    int height = inImg.size().height;
    int width = inImg.size().width;
    int rc, k;
    Img myImg;
    myImg.inImg = inImg;

    cout << "W: " << width << endl;
    cout << "H: " << height << endl;
   
    int hx = height/h;
    int wx = width/w;

    //std::thread t[hx*wx];

    cout << hx << endl;
    cout << wx << endl;

    int wxx = w/wx, hxx = h/hx;

    k = 0;
    for (int i = 0; i < hx; i++) {
        for (int j = 0; j < wx; j++) {
            myResize2(inImg, outImg, i, j, w, h, wx, hx, wxx, hxx);
           // t[k] = std::thread(myResize, std::ref(inImg), std::ref(outImg), i, j, w, h, wx, hx, wxx, hxx);   
            k++;
        }
    }
    //int i = 0, j = 0;


    //std::thread second(bar, std::ref(k)); 

    //for (int i = 0; i < k; i++)
    //   t[i].join();
    //outImg = myImg.outImg;
    //second.join();
    //cout << k << endl;
}


// void processImage(Mat &inImg, Mat &outImg)
// {
//     int height = inImg.size().height;
//     int width = inImg.size().width;

//     for (int i = 0; i < height; i += 128) {
//         for (int j = 0; j < width; j+= 128) {
//             Mat block = inImg(Rect(j, i, 128, 128));
//             resize(block, block, Size(), 0.5, 0.5, CV_INTER_LINEAR);
//             block.copyTo(outImg(Rect(j/2,i/2,64, 64)));
//         }
//     }
// }
