/**
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *
 * Created by Felipe Rodriguez Arias <ucifarias@gmail.com> on 04/12/2015.
 */

#include <QCoreApplication>
#include <stdio.h>

#include <opencv/cv.h>
#include <opencv2/opencv.hpp>

using namespace cv;
void my_mouse_callback(int event, int x, int y, int flags, void* param);
CvRect box;
bool drawing_box = false;

// RUTINA PARA DIBUJAR DENTRO EN LA IMAGEN
void draw_box( IplImage* img, CvRect rect )
{
    cvRectangle (
                img,
                cvPoint(box.x,box.y),
                cvPoint(box.x+box.width,box.y+box.height),
                cvScalar(0xff,0x00,0x00)    /* red */
                );
}

void my_mouse_callback(int event, int x, int y, int flags, void* param)
{
    IplImage* image = (IplImage*) param;

    switch( event ) {
    case CV_EVENT_LBUTTONUP:{
        box.width=60;//80
        box.height=60;//80
        box.x=x-box.width/2;
        box.y=y-box.height/2;
    }
        draw_box(image, box);
        break;

    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //DECLARACION DE LA IMAGEN QUE SE MOSTRARA COMO VIDEO NO ESTABILIZADO
    IplImage *imgActual_IPL;//para poder terminar el video sin error con if(!imgActual_IPL) ya que if(!imgActual.data) da error.
    Mat imgActual_MAT;
    CvCapture *Capture;
    Capture = cvCreateFileCapture("shaky_car.avi");
    //Capture = cvCreateFileCapture("rtsp://10.80.72.197:554/ISAPI/streaming/channels/2?auth=YWRtaW46SHVhV2VpMTIz");
    imgActual_IPL=cvQueryFrame(Capture);
    //cv::cvtColor(imgActual_IPL,imgActual_MAT,CV_BayerRG2GRAY);
    imgActual_MAT=imgActual_IPL;

    //DECLARACION DE LAS VARIABLES QUE PARTICIPAN EN EL DIBUJO DE LA REGION DE INTERES
    box=cvRect(/*80,80,50,50/**/70,60,80,80);
    IplImage* image =imgActual_IPL;
    IplImage* temp = cvCloneImage( image );
    cvNamedWindow( "Box Example" );
   // cv::namedWindow("prueba");
    cvSetMouseCallback("Box Example",my_mouse_callback,(void*) imgActual_IPL);
    bool flag=false;

    //DECLARACION DE LAS FILAS Y COLUMNAS QUE SE TOMARAN DE LA IMAGEN DE VIDEO PARA EL RELLENO
    int starX = 0,
            starY = 0,
            endX  = imgActual_MAT.cols,
            endY  = imgActual_MAT.rows,
            poscBackGroundY=0,
            poscBackGroundX=0;

    //ESTA ES LA REGION CAMBIANTE QUE SE TOMA DE LA IMAGEN QUE SE ESTA ANALIZANDO Y MOSTRANDO
    //EN EL MOMENTO,LA CUAL SE LE SUMA A backGroundROI Y SE MUESTRA COMO EL VIDEO FINALMENTE
    //ESTABILIZADO EN backGround
    Mat imageROI = imgActual_MAT.colRange(starX,endX);

    imageROI = imgActual_MAT.rowRange(starY,endY);

    //DECLARACION DE LA REGION DE INTERES QUE SE TOMA DEL VIDEO NO ESTABILIZADO
    //PARA PROCESAR.EN ESTE CASO SERIA LA PARTE DONDE SE ENCUENTRA EL AUTOMOVIL
    //(PREDETERMINADO EN EL CENTRO DE LA IMAGEN).EL USARIO PODRÁ ELEGIRLA CON EL MOUSE
    int width=  box.width,
            height= box.height,
            poscInicX = box.x,
            poscInicY = box.y,
            deltaX=0,
            deltaY=0,
            c=0,
            frames=1;

    //REGION DE LA IMAGEN NO ESTABILIZADA QUE SERA PROCESADA
    Mat imgActualROI=imgActual_MAT(cv::Rect(poscInicX,poscInicY,width,height));

    //INICILALIZACION DE LA CLASE QUE SE ENCARGA DE DETECTAR LOS PUNTOS DE INTERES.
    std::vector<cv::KeyPoint> gftt_keypoints1,gftt_keypoints2;
    cv::GoodFeaturesToTrackDetector gftt(
                500,  //maximo numeros de puntos a detectar
                0.01, // calidad
                10,   // distancia minima permitida
                7,	// BlockSize(su dimension determina que el primeros de todos los ptos sea Dx siempre en la misma posc. en todos los frames)
                true);// uso del Dx de esquina de Harris(junto con BlockSize determina que el primeros de todos los ptos sea Dx siempre en la misma posc. en todos los frames)

    //DETERMINACION DEL PRIMER PUNTO DETECTADO EL CUAL SE TOMA CON REFERENCIA
    //PARA SABER CUANTO SE DESPLAZA EN EL PROXIMO FRAME Y ASI ESTABILIZAR EL VIDEO
    int offSetX=0;
    int offSetY=0;

    //DECLARACION DEL BACKGROUN
    Mat backGroundExtended,backGround,backGroundROI;
    
 //   CvCapture *CaptureBack;
 //   CaptureBack = cvCreateFileCapture("background.avi");
 //   backGroundExtended=cvQueryFrame(CaptureBack);
 //   backGround= backGroundExtended(cv::Rect(1,1,imgActual_MAT.cols,imgActual_MAT.rows));
 //   backGround= backGround*0;


    //POSICION CAMBIANTE DE LA REGION DEL BACKGROUND DONDE SE MOSTRARÁ LA IMAGEN ESTABILIZADA
    //LA CUAL SE SUMA A imageROI
    //backGroundROI = backGround(cv::Rect(0,0,imgActual_MAT.cols,imgActual_MAT.rows));
    backGroundROI =  cv::Mat::zeros(imgActual_MAT.rows,imgActual_MAT.cols, imgActual_MAT.type());
   // imshow("prueba", backGroundROI);

    //REGION DE LA IMAGEN ANTERIOR QUE SE TOMARA PARA DETERMINAR SU PUNTO MAS NOTABLE
    //EL CUAL SE SUPONE QUE CAMBIE DE POSCION EN EL SIGUIENTE FRAME(DEFINO EN:imgActualROI)
    Mat imgAnteriorROI;
    int tempBoX=box.x;
    CvRect trackRect=box;
    Mat imgActualROI2=imgActualROI;


    while(true)
    {
        cvCopyImage( image, temp );
        if( drawing_box ) draw_box( temp, box );

        imgAnteriorROI=imgActualROI2;
        gftt.detect(imgAnteriorROI, gftt_keypoints1);

here: imgActual_IPL=cvQueryFrame(Capture);
        if(!imgActual_IPL)break;
        imgActual_MAT=imgActual_IPL;
        imgActualROI=imgActual_MAT(cv::Rect(trackRect.x,trackRect.y,trackRect.width,trackRect.height));
        gftt.detect(imgActualROI,gftt_keypoints2);

        //Aqui se calcula que desplazamiento que exite entre el frame enterior y el siguiente
        deltaX=gftt_keypoints2[0].pt.x-gftt_keypoints1[0].pt.x;
        deltaY=gftt_keypoints2[0].pt.y-gftt_keypoints1[0].pt.y;

        //si se hace click izquierdo selecciona otra region y se actualizan las posiciones
        if(tempBoX!=box.x)
        {
            if(box.x<=1 || box.y<=1 || box.x+box.width>= imgActual_MAT.cols || box.y+box.height>=imgActual_MAT.rows) box=cvRect(70,60,80,80);
            tempBoX=box.x;
            trackRect=box;

            imgAnteriorROI=imgActual_MAT(cv::Rect(trackRect.x,trackRect.y,trackRect.width,trackRect.height));
            imshow("new box",imgAnteriorROI);

            gftt.detect(imgAnteriorROI,gftt_keypoints1);

            goto here;
        }

        //Este es el rectangulo que sigue al auto
        trackRect.x=trackRect.x+deltaX;
        trackRect.y=trackRect.y+deltaY;
        cvRectangleR(imgActual_IPL,trackRect,cvScalar(0,255,0),1);


        if(trackRect.x<=10 || trackRect.y<=10 || 
        (trackRect.x+trackRect.width)>= (imgActual_MAT.cols-10) || 
        (trackRect.y+trackRect.height) >= (imgActual_MAT.rows-10))
        {
            trackRect=cvRect(70,60,80,80);    
        }        
        
        imgActualROI2=imgActual_MAT(cv::Rect(trackRect.x,trackRect.y,trackRect.width,trackRect.height));
        //este es la distancia que se desplaza el carro siempre alrededor de posicion inicial
        offSetX=trackRect.x-box.x;
        offSetY=trackRect.y-box.y;

        if(offSetX>=0)//desplazamiento hacia la derecha
        {
            starX=abs(offSetX);
            endX=imgActual_MAT.cols;
            poscBackGroundX=0;

        }
        else         //desplazamiento hacia la izquierda
        {
            starX=0;
            endX=imgActual_MAT.cols+offSetX;
            poscBackGroundX=abs(offSetX);
        }

        if(offSetY>=0)//desplazamiento hacia abajo
        {
            starY=abs(offSetY);
            endY=imgActual_MAT.rows;
            poscBackGroundY=0;

        }
        else          //desplazamiento hacia arriba
        {
            starY=0;
            endY=imgActual_MAT.rows+offSetY;
            poscBackGroundY=abs(offSetY);
        }

        imageROI = imgActual_MAT.colRange(starX,endX);
        imageROI = imageROI.rowRange (starY,endY);

       //backGroundROI = backGround(cv::Rect(poscBackGroundX,poscBackGroundY,imageROI.cols,imageROI.rows));
        backGround = cv::Mat::zeros(imgActual_MAT.rows ,imgActual_MAT.cols,imageROI.type());
        backGroundROI = backGround(cv::Rect(poscBackGroundX,poscBackGroundY,imageROI.cols,imageROI.rows));
        cv::addWeighted(imageROI,1,backGroundROI,0.0,0.,backGroundROI);

        imshow("Video Estabilizado",backGround);
       // backGround=backGround*0;

        cvShowImage( "Box Example", temp );

        frames++;
        std::cout<<frames<<'\t'<<trackRect.x<<'\t'<<trackRect.y<<'\t'<<'*'<<'\t'<<deltaX<<'\t'<<deltaY<<'\t'<<std::endl;

        if(cvWaitKey( 100 )==27)break;

    };

    system("pause");
    cvReleaseCapture(&Capture);

    return a.exec();
}
