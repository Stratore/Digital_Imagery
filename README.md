# Digital Imagery

Petite bibliothèque de traitement d'images en C++, écrite sans aucune dépendance :
une image générique, des images en niveaux de gris et en couleur, et la lecture /
écriture des formats PGM, PPM et TGA.

![C++](https://img.shields.io/badge/C%2B%2B17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Statut](https://img.shields.io/badge/statut-en%20cours-lightgrey?style=flat-square)

## Contexte

Projet de cours du BUT Informatique (IUT d'Arles), **en cours** : les formats
standards fonctionnent, deux parties ne sont pas encore terminées (voir plus bas).

## Ce que fait la bibliothèque

**`Image<PixelType>`**, une image générique (template) :

- allocation, copie, accès à un pixel avec contrôle des bornes (exception en dehors
  de l'image)
- remplissage, rectangle vide ou plein
- redimensionnement au plus proche voisin (`simpleScale`) ou bilinéaire
  (`bilinearScale`)

**`GrayImage`** (niveaux de gris, un octet par pixel) :

- PGM en lecture et écriture
- TGA non compressé en niveaux de gris en lecture et écriture

**`ColorImage`** (pixels `Color` RGB) :

- PPM en lecture et écriture
- TGA en écriture, brut ou compressé RLE
- TGA en lecture, couleurs directes ou image à palette

**`Color`** : couleur RGB avec addition, mélange pondéré et conversion depuis CMY.

## En cours

- **Format maison** (`readMaison`) : lecture d'un format d'image propre au cours,
  en CMY, pas encore terminée
- **Anaglyphe** (image 3D rouge / cyan) : commencé, pas encore fonctionnel

## Compiler

Aucune dépendance, un compilateur C++17 suffit :

```bash
g++ -std=c++17 -Wall -c src/Image.cpp
```

Il n'y a pas encore de programme de démonstration : la bibliothèque s'utilise en
incluant `src/Image.hpp`. Deux images d'essai sont fournies dans `samples/`.

## Organisation du dépôt

```
src/Image.hpp        classes Color, Image<T>, GrayImage, ColorImage
src/Image.cpp        lecture / écriture des formats, conversions
samples/             images d'essai (chat.pgm, chat_petit.ppm)
```
