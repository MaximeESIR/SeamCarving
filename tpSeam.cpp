#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "imagesRW.h"
#include <iostream>
#include <string.h>

void filtrage2D(const unsigned char *I, double *Ic, double *K, int sizeX, int sizeY)
{
    int offset = 1; // taille du filtre / 2
    double val = 0;

    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j < sizeY; j++)
        {
            int coord = i * sizeY + j;
            val = 0; // Valeur de I'[i,j]

            for (int s = 0; s < 3; s++)
            {
                for (int t = 0; t < 3; t++)
                {
                    int coordK = s * 3 + t;
                    int imgX = i - offset + s;
                    int imgY = j - offset + t;

                    // Gestion des bords de l'image avec symétrie
                    if (imgX < 0)
                    {
                        imgX = -imgX;
                    }
                    else if (imgX >= sizeX)
                    {
                        imgX = 2 * sizeX - imgX - 2;
                    }

                    if (imgY < 0)
                    {
                        imgY = -imgY;
                    }
                    else if (imgY >= sizeY)
                    {
                        imgY = 2 * sizeY - imgY - 2;
                    }

                    val += K[coordK] * I[imgX * sizeY + imgY];
                }
            }

            if (val > 255)
            {
                Ic[coord] = 255;
            }
            else
            {
                Ic[coord] = val;
            }
        }
    }
}



// Calcul du gradient de imgIn $
void IMgradient(unsigned char *imgIn, unsigned char *imgOut, double *photoGradient, int sizeX, int sizeY)
{
    double Gx[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    double Gy[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

    double *photoGx = new double[sizeX * sizeY]; // Photo du gradient suivant X
    double *photoGy = new double[sizeX * sizeY]; // Photo du gradient suivant Y

    filtrage2D(imgIn, photoGx, Gx, sizeX, sizeY);
    filtrage2D(imgIn, photoGy, Gy, sizeX, sizeY);

    for (int i = 0; i < sizeX * sizeY; i++)
    {
        photoGradient[i] = std::sqrt(photoGx[i] * photoGx[i] + photoGy[i] * photoGy[i]) / 2;
    }
    for (int i = 0; i < sizeX * sizeY; i++)
    {
        imgOut[i] = (unsigned char)(photoGradient[i]);
    }
}

// Recherche du chemin d'energie minimale dans imgIn
void min_path(double *photoCumule, double *photoGradient, unsigned char *photoSortieCum, char *filesortie,
         unsigned char *photo, int sizeX, int sizeY, int *chemin)
{
    ////////////////////////// Energie cumulée ////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Initialisation premiére colonne de l'image resultat
    double maxi = 0;
    for (int i = 0; i < sizeX; i++)
    {
        photoCumule[i * sizeY] = photoGradient[i * sizeY];
    }
    // On rempli le reste des colonnes
    for (int j = 1; j < sizeY; j++)
    {
        for (int i = 0; i < sizeX; i++)
        {
            double mini = photoCumule[i * sizeY + j - 1];
            for (int k = 0; k < 3; k++)
            {
                int voisins = (i - 1 + k) * sizeY + j - 1;
                if (voisins > 0 && voisins < sizeX * sizeY && photoCumule[voisins] < mini)
                {
                    mini = photoCumule[voisins];
                }
            }
            photoCumule[i * sizeY + j] = mini + photoGradient[i * sizeY + j];
            // pour rescale
            if (photoCumule[i * sizeY + j] > maxi)
            {
                maxi = photoCumule[i * sizeY + j];
            }
        }
    }

    // On converti en unsigned char pour pouvoir afficher sur l'image sortie
    for (int i = 0; i < sizeX * sizeY; i++)
    {
        photoSortieCum[i] = (unsigned char)(photoCumule[i] * 255 / maxi);
    }
    strcpy(filesortie, "./images/loupCumul.pgm");
    writePGM_Picture(filesortie, photoSortieCum, sizeY, sizeX);

    // Chemin d'énergie minimale ////////////////////////////////////:

    int indice_min = 0; // Indice de la ligne

    // On ch sur la derniere colonne par ou commencer !
    for (int i = 0; i < sizeX; i++)
{   
    if (photoCumule[i * sizeY + sizeY - 1] < photoCumule[indice_min * sizeY + sizeY - 1])
    {
        indice_min = i; // 0 ou... 425
    }
}


    chemin[sizeY - 1] = indice_min; // Initialisation
    // trouver le reste du chemin min
    for (int j = sizeY - 2; j >= 0; j--)
    {
        int ancienneCoord = chemin[j + 1]; // Coordonée du point précédent ie i
        int coord = ancienneCoord;         // coordonée de la couture pour cette colone
        double mini = photoCumule[ancienneCoord * sizeY + j - 1];

        for (int k = 0; k < 3; k++)
        {
            int voisin = (ancienneCoord - 1 + k) * sizeY + j - 1;
            if (voisin > 0 && voisin < sizeX * sizeY && photoCumule[voisin] < mini)
            {
                mini = photoCumule[voisin];
                coord = ancienneCoord - 1 + k;
            }
        }
        chemin[j] = coord;
    }

}

void suppr_couture(unsigned char *imgIn, unsigned char *imgOut, int sizeX, int sizeY, int *chemin)
{
    for (int j = 0; j < sizeY; j++)
    {
        for (int i = 0; i < sizeX; i++)
        {
            if (i < chemin[j])
            {
                imgOut[i * sizeY + j] = imgIn[i * sizeY + j];
            }
            else
            {
                imgOut[i * sizeY + j] = imgIn[(i + 1) * sizeY + j];
            }
        }
    }
}

int main(int argc, char **argv)
{
    ////////////////////////// Initialisation ////////////////////////////
    ///////////////////////////////////////////////////////////////////////
     int sizeX = 425;      // Nombre de lignes
    const int sizeY = 290;      // Nombre de colonnes
    char filename[256];   // Nom du fichier à charger
    char filesortie[256]; // Nom du fichier en sortie

    strcpy(filename, "./images/loup.pgm");
    strcpy(filesortie, "./images/loupCumul.pgm");

    unsigned char *photo = new unsigned char[sizeX * sizeY];       // Photo entrée
    unsigned char *photoSortie = new unsigned char[sizeX * sizeY]; // Photo sortie

    double *photoGradient = new double[sizeX * sizeY]; // Photo de la norme du gradient
    double *photoCumule = new double[sizeX * sizeY];   // Photo de la matrice d'accumulmation des energies horizontale gauche/droite


    int chemin[sizeY]; // stockage des indices
    readPGM_Picture(filename, photo, sizeX, sizeY); //lecture de l'image
    IMgradient(photo, photoSortie, photoGradient, sizeX, sizeY); //Calcul du gradient
    strcpy(filesortie, "./images/louGradient.pgm");
    writePGM_Picture(filesortie, photoSortie, sizeY, sizeX);

    // min path
    min_path(photoCumule, photoGradient, photoSortie, filesortie, photo, sizeX, sizeY, chemin);
    unsigned char *imgOut = new unsigned char[(sizeX-1) * sizeY];
    suppr_couture(photo, imgOut, sizeX-1, sizeY, chemin);   
    
    for (int k = 0; k < 100; k++)
    {
        sizeX--;
        photoSortie = new unsigned char[sizeX * sizeY]; // Photo sortie
        photoGradient = new double[sizeX * sizeY];             // Photo de la norme du gradient
        photoCumule = new double[sizeX * sizeY];               // Photo de la matrice d'accumulmation des energies horizontale gauche/droite
        IMgradient(imgOut, photoSortie, photoGradient, sizeX, sizeY);

        strcpy(filesortie, "./images/louGradient.pgm");
        writePGM_Picture(filesortie, photoSortie, sizeY, sizeX);

        // min path
        min_path(photoCumule, photoGradient, photoSortie, filesortie, photo, sizeX, sizeY, chemin);
       
        unsigned char *imgTmp = new unsigned char[(sizeX-1) * sizeY];
        suppr_couture(imgOut, imgTmp, sizeX-1, sizeY, chemin);
        delete[] imgOut;
        imgOut = new unsigned char[(sizeX-1) * sizeY];
        for(int j=0;j<sizeY*(sizeX-1);j++){
        imgOut[j]=imgTmp[j]; //voir si ca marche hein
        }
        delete[] imgTmp;
        delete[] photoCumule;
        delete[] photoSortie;
        delete[] photoGradient;
    }

    strcpy(filesortie, "./images/sortieFinaleLoup.pgm");
    writePGM_Picture(filesortie, imgOut, sizeY, sizeX);
    delete imgOut;
    
    // l'enlever de l'image
    //test en affichant du blanc sur le chemin minimal
    for(int j=0;j<sizeY;j++){
        photoSortie[chemin[j]*sizeY +j]=255; //voir si ca marche hein
    }
    strcpy(filesortie, "./images/loupTrace.pgm");
    writePGM_Picture(filesortie, photoSortie, sizeY, sizeX);
    

    delete photo;

    return 0;
}
