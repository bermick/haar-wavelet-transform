#include <iostream>
#include <stdio.h>
#include <math.h>
#include "Imagen.h"


using namespace std;

Imagen::Imagen(){
	this->tamano = 0;
}

void Imagen::guardarBMP(const char* filename, double* matriz)
{
    unsigned char* tmp = new unsigned char[tamano]; 

    for(int i=0; i<tamano; i++) tmp[i] = (unsigned char) matriz[i];

    FILE* f = fopen(filename, "wb");


    fwrite(header, sizeof(unsigned char), 54, f); // 

    fwrite(tmp, sizeof(unsigned char), tamano, f);


    fclose(f);
}

double* Imagen::leerBMP(const char* filename)
{
    FILE* f = fopen(filename, "rb");


    fread(header, sizeof(unsigned char), 54, f); // leer el header de 54 bytes

    // extraer ancho y alto
    int ancho = *(int*)&header[18];
    int alto = *(int*)&header[22];

    setTamano(3 * ancho * alto);
    unsigned char* data = new unsigned char[tamano]; // 3 bytes por pixel
    fread(data, sizeof(unsigned char), tamano, f); // leer el cuerpo de la imagen
    fclose(f);

    double *imagen = new double[tamano];

    for(int i=0; i<tamano; i++) imagen[i] = (double) data[i];


    delete[] data;
    return imagen;
}

//Separa la imagen en los tres canales, en 3 distintos apuntadores
void Imagen::separarYUV(double* imagen)
{
	unsigned t = getTamano()/3;
	unsigned a;
    unsigned renglones;
    this->Y = new double[t];
    this->U = new double[t];
    this->V = new double[t];

    t = sqrt(t);
    renglones = t*3;
	//separar los 3 canales
    for(int i=0; i<t; i++)
    {
        a=0;
        for(int j=0; j<renglones; j+=3)
        {
            this->V[i*t + a] = imagen[i*renglones + j];
            this->U[i*t + a] = imagen[i*renglones + j+1];
            this->Y[i*t + a] = imagen[i*renglones + j+2];
            a++;
        }
    }

}

void Imagen::unirYUV(double* imagen)
{
    unsigned t = this->tamano;
    unsigned a = 0;

    for(int i = 0; i<t; i+=3){
        imagen[i] = (char) escalar(-1, 1, 0, 255, this->V[a]);
        imagen[i+1] = (char) escalar(-1, 1, 0, 255, this->U[a]);
        imagen[i+2] = (char) escalar(-1, 1, 0, 255, this->Y[a]);

        a++;
    }
}


//Separa la imagen en los tres canales dentro de un mismo vector
// BBBGGGRRR
double* Imagen::separarRGB(double* imagen)
{
    unsigned t = getTamano();
    unsigned offset = t/3;
    double * rgb;
    rgb = new double[t];
    int i;
    for( i = 0; i < offset; i++){
        rgb[i] = escalar(0, 255, -1, 1, imagen[i*3]);//B
        rgb[i+offset] = escalar(0, 255, -1, 1, imagen[(3*i)+1]);//G
        rgb[i+(offset*2)] = escalar(0, 255, -1, 1, imagen[(3*i)+2]);//R
    }
    return rgb;
}

double* Imagen::unirRGB(double* imagen)
{
    unsigned t = this->tamano/3;
    double * rgb;

    rgb = new double[tamano];
    int i; int a = 0;
    for( i = 0; i<tamano; i+=3){
        rgb[i] = (char) escalar(-1, 1, 0, 255,  imagen[a]);
        rgb[i+1] =(char) escalar(-1,1,0, 255,  imagen[a+t]);
        rgb[i+2] = (char) escalar(-1,1,0,255, imagen[a+(t*2)]);

	a++;
    }

    return rgb;
}

void Imagen::setTamano(int t)
{
	this->tamano = t;
}


int Imagen::getTamano()
{
	return this->tamano;
}

double Imagen::escalar(double fromMin, double fromMax, double toMin, double toMax, double x)
{
    if (fromMax - fromMin == 0) return 0;
    double value = (toMax - toMin) * (x - fromMin) / (fromMax - fromMin) + toMin;
    if (value > toMax)
        value = toMax;
    if (value < toMin)
        value = toMin;
    return value;
}
