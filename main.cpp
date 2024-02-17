#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>
#include "SerialClass.h"

using namespace cv;
using namespace std;



//Canny treshold
int thresh = 40;

//Random number generator
RNG rng(12345);

int main(int argc, char* argv[])
{
 
    cv::VideoCapture camera(0);
    if (!camera.isOpened()) {
        std::cerr << "ERROR: Could not open camera" << std::endl;
        return 1;
    }



    // this will contain the image from the webcam
    Mat frame;

    // capture the next frame from the webcam
    camera >> frame;
    cout << "Width0: " << frame.size().width << "Height0: " << frame.size().height  <<"\n";


    
    Mat img = frame;
    // Crop image
    Mat cropped_image = img(Range(50, 480), Range(90, 600)); //top-> bottom
    img = cropped_image;
    cout << "Width: " << img.size().width << "Height: " << img.size().height << "\n";
    
    //Filtering Image
    Mat img_gray;
    cvtColor(img, img_gray, COLOR_BGR2GRAY);
    blur(img_gray, img_gray, Size(3, 3));


    //canny edge detection
    Mat canny_output;
    Canny(img_gray, canny_output, thresh, thresh * 2);
    imshow("Canny output", canny_output);

    //Finding Contours
    vector<vector<Point> > contours;
    findContours(canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
    vector<vector<Point> > contours_poly(contours.size());

  

    //vectors to make rectangles and circles around detected object
    vector<Rect> boundRect(contours.size());
    vector<Point2f>centers(contours.size());
    vector<float>radius(contours.size());

    //Approximating the contours to more simple lines
    for (size_t i = 0; i < contours.size(); i++)
    {
        approxPolyDP(contours[i], contours_poly[i], arcLength(contours[i], true) * 0.02, true);
        boundRect[i] = boundingRect(contours_poly[i]);
        minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
    }

    //Delete small rectangles (false positives)
    for (int i = 0; i < boundRect.size(); i++)
        if (boundRect[i].height < 10 || boundRect[i].width < 10)
            boundRect.erase(boundRect.begin()+ i);

    
    //Draw edges
    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
    Scalar color(rand() & 255, rand() & 255, rand() & 255);
    drawContours(drawing, contours_poly, -1, color);

    //Color conversion
    Mat hsvImg;
    cvtColor(img, hsvImg, COLOR_BGR2HSV);

    vector<int> centers_arduino;

    //Select color
    int selectedColor;
    cout << "Select the color: 0 for red, 1 for blue" << endl;
    cin >> selectedColor;
    
    vector<Scalar> temp_avg_color;
    vector<int> temp_coordinates;
    //Threshold values
    Scalar lower_red(10, 85, 40);
    Scalar upper_red(50, 255, 105);
    Scalar lower_blue(55, 65, 10);
    Scalar upper_blue(100, 250, 75);
    for (size_t i = 0; i < boundRect.size(); i++) 
    {
        if (boundRect[i].width > 60 && boundRect[i].height > 60)
        {
            //Draw rectangle
            Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
            rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2);
            //Add center
            int center_add = boundRect[i].tl().x + boundRect[i].width / 2;
            Mat roi = hsvImg(boundRect[i]);
            imshow("roi", roi);
            Scalar avg = mean(roi);
        
            if (selectedColor == 0)
            {
                
                if (avg[0] >= lower_red[0] && avg[0] <= upper_red[0] && avg[1] >= lower_red[1] && avg[2] >= lower_red[2]) //HSV!!
                {
                    centers_arduino.push_back(center_add);
                }
                else
                {
                    temp_avg_color.push_back(avg);
                    temp_coordinates.push_back(center_add);
                }
            }
            if (selectedColor == 1)
            {                
                if (avg[0] >= lower_blue[0] && avg[0] <= upper_blue[0] && avg[1] >= lower_blue[1] && avg[1] <= upper_blue[1] && avg[2] >= lower_blue[2] && avg[2] <= upper_blue[2]) //HSV!!
                    centers_arduino.push_back(center_add);
                else
                {
                    temp_avg_color.push_back(avg);
                    temp_coordinates.push_back(center_add);
                }
            }
  
        }
    }
    imshow("Contours", drawing);

    //Rearrange if we did not find anything
    int selected_idx = 0;
    if (centers_arduino.empty())
    {
        if(selectedColor == 0)
        {
            //Substitute the values with their offset
            for (int i = 0; i < temp_avg_color.size(); i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    int min = abs(temp_avg_color[i][j] - lower_red[j]);
                    int max = abs(temp_avg_color[i][j] - upper_red[j]);

                    if (min < max)
                        temp_avg_color[i][j] = min;
                    else
                        temp_avg_color[i][j] = max;
                }
            }
            //Compare the offset and select the lower one
            for (int i = 0; i < temp_avg_color.size(); i++)
            {
                int trashold_offset = 0;
                for (int j = 0; j < 3; j++)
                {
                    if (temp_avg_color[i][j] < temp_avg_color[selected_idx][j])
                        trashold_offset++;
                }

                if (trashold_offset >= 2)
                    selected_idx = i;
            }

                centers_arduino.push_back(temp_coordinates[selected_idx]);
            
        }else
        {
            //Substitute the values with their offset
            for (int i = 0; i < temp_avg_color.size(); i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    int min = abs(temp_avg_color[i][j] - lower_blue[j]);
                    int max = abs(temp_avg_color[i][j] - upper_blue[j]);

                    if (min < max)
                        temp_avg_color[i][j] = min;
                    else
                        temp_avg_color[i][j] = max;
                }
            }
            //Compare the offset and select the lower one
            for (int i = 0; i < temp_avg_color.size(); i++)
            {
                int trashold_offset = 0;
                for (int j = 0; j < 3; j++)
                {
                    if (temp_avg_color[i][j] < temp_avg_color[selected_idx][j])
                        trashold_offset++;
                }

                if (trashold_offset >= 2)
                    selected_idx = i;
            }
                centers_arduino.push_back(temp_coordinates[selected_idx]);
        }
       
    }

    //Send the mean value
    int send_center = 0;
    if (!centers_arduino.empty())
    {
        send_center = centers_arduino[0];
        for (int i = 0; i < centers_arduino.size(); i++)
        {
            if (i != 0)
            {
                send_center = (send_center + centers_arduino[i]) / 2;
            }
        }
    }
    

    //Conversion to char
    string string_send_center = to_string(send_center);
    char a[3];
    for (int i = 0; i < string_send_center.size(); i++)
    {
        a[i] = string_send_center[i];
    }
    


    //Sending char via serialport  
    Serial* SP = new Serial("\\\\.\\COM3");    // adjust as needed
    Sleep(100);
    if (SP->IsConnected())
    string myString = to_string(send_center);
    myString.push_back('a');

    char* char_array = new char[myString.length()+1];
    char_array[myString.length()] = '\0';

    for (int i = 0; i < myString.length(); i++) {
        char_array[i] = myString[i];
    }
    cout << char_array <<endl;
    bool is_send = false;
    is_send = SP->WriteData(char_array,strlen(a));
    std::cout << is_send << "\n";
    waitKey(0);
    return 0;
}

