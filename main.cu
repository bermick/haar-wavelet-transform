#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include "Imagen.h"
#include <stdlib.h>
#include <stdio.h>
#include <cuda_runtime.h>

// t actual size of the image: 1024, 512, 256
// s es el ancho de la imagen inicial
__global__ void
HWT_1D_h( double * e, double * sal, int s, int t )
{
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;
	int dz = blockIdx.z * (s*s) ;
	//si excedimos la mitad horizontal o todas las filas
	if( x >= ( t + 1) / 2 || y >= t ) return;
	
	const double sqrt2 = 2;//1.414213;
	//s es el ancho de la imagen
	int i = ( y * s + 2 * x ) + dz ;//el *2 es sporque se avanza de dos en dos: 0,2,4,6, debo agarrar la x-ésima pareja de números
	int ii = ( y * s + x) + dz;//posición exacta dentro de la matriz gral
	
	sal[ii] = (e[i] + e[i+1])/sqrt2;
	sal[ii+t/2] = (e[i] - e[i+1])/sqrt2;

}


__global__ void
HWT_1D_v ( double * e, double * sal, int s, int t)
{
	int y = blockIdx.x * blockDim.x + threadIdx.x;
	int x = blockIdx.y * blockDim.y + threadIdx.y;
	int dz = blockIdx.z * (s*s) ;

	if(y >= (t+1)/2 || x >= t ) return;

	int o = (2 *  y * s + x) + dz ;
	int p = o + s;
	int q = y*s+x + dz;

	const double sqrt2 = 2;// 1.4142135;
	
	sal[q] = (e[o] + e[p]) / sqrt2;
	sal[q+s*t/2] = (e[o] - e[p])/sqrt2;
}


//t = tamaño total n*n*3
void HWT_2D(double* h_imagen , int t, int n){
	double* d_e;
	double* d_s;

	//tamaño total  en bytes de la imagen
	int tam_i = t * sizeof(double);
	

	cudaMalloc(&d_e, tam_i);
	cudaMalloc(&d_s, tam_i);

	
	cudaMemcpy(d_e, h_imagen , tam_i, cudaMemcpyHostToDevice);

	dim3 bloque(32,32);//1024 maxThreadsPerBlock
	dim3 grid;

	t /= 3;//tamano de cada matriz
	t = sqrt(t);//ancho de la imagen

	grid.x = ( t + bloque.x - 1) / bloque.x;//~~32
	grid.y =(t + bloque.y - 1) / bloque.y;//~~32
	grid.z = 3;

	grid.x *= 0.5;//solo se necesitan n/2 hilos

	int i = t;
	
	float tiempo;
	cudaEvent_t inicio, fin;
	cudaEventCreate(&inicio);
	cudaEventCreate(&fin);

	cudaEventRecord(inicio, 0);

	while(i > 1 && n > 0 ){
		HWT_1D_h<<<grid,bloque>>>(d_e, d_s, t, i);
		cudaDeviceSynchronize();
		HWT_1D_v<<<grid, bloque>>>(d_s, d_e, t, i);
		cudaDeviceSynchronize();
		i /= 2;
		n--;
	}

	cudaEventRecord(fin, 0);
	cudaEventSynchronize(fin);
	cudaEventElapsedTime(&tiempo, inicio, fin);

	printf("Tiempo de transformación paralelo: %3.2f ms \n", tiempo);
	
	cudaMemcpy(h_imagen, d_e, tam_i, cudaMemcpyDeviceToHost);
	
	cudaFree(d_e); cudaFree(d_s);
}

 

int main(int argc, char **argv){
    if(argc<4) { printf("Uso: ./ejecutable Npasos entrada salida\n");   exit(0);}

    double * matriz;
    double * vectorBGR;
    int n = atoi(argv[1]);
    int t;
    char *entrada = argv[2];	
    char *salida = argv[3];	
    Imagen * imagen;

    imagen = new Imagen();

    //obtener matriz de imagen
    matriz = imagen->leerBMP(entrada);

    //obtener tamano de la imagen
    t = imagen->getTamano();
    
    vectorBGR = imagen->separarRGB(matriz);

    HWT_2D(vectorBGR, t, n);

    matriz = imagen->unirRGB(vectorBGR);

    imagen->guardarBMP(salida, matriz);
    return 0;
}
