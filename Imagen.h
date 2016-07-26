class Imagen{

public:
	Imagen();
	//regresa apuntador a stream de imagen
	double* leerBMP(const char* );
	void guardarBMP(const char*, double*);
	
	//separa en su cpdte apuntador cada uno de los 3 canales
	void separarYUV(double*);
	void unirYUV(double*);

	void setTamano(int);
	int getTamano();
	
	double* separarRGB(double*);
	double* unirRGB(double*);

	double escalar(double, double, double, double, double);

	double* Y;
	double* U;
	double* V;

    unsigned char header[54];

private:
	int tamano;
		
};
