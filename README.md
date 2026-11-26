# Digital Imagery

Petite bibliothèque de traitement d'images en C++, écrite sans aucune dépendance :
une image générique, des images en niveaux de gris et en couleur, la lecture /
écriture des formats PGM, PPM et TGA, un format maison en CMY et l'anaglyphe.

![C++](https://img.shields.io/badge/C%2B%2B17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Statut](https://img.shields.io/badge/statut-fonctionnel-brightgreen?style=flat-square)

## Contexte

Projet de cours du BUT Informatique (IUT d'Arles). Toutes les parties
fonctionnent : formats standards, format maison et anaglyphe.

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
- **format maison** (`readMaison`) : un format propre au cours, avec un en-tête
  de 9 octets (`Maison`, longueur du commentaire, largeur, hauteur), le
  commentaire, puis les pixels en CMY, convertis en RGB
- **anaglyphe** (`anaglyphe`) : à partir d'une image qui contient les deux vues
  côte à côte, une image 3D à regarder avec des lunettes rouge / cyan (rouge de
  la vue de gauche, vert et bleu de la vue de droite)

**`Color`** : couleur RGB avec addition, mélange pondéré et conversion depuis CMY.

Les erreurs (fichier tronqué, en-tête invalide, accès hors de l'image) lèvent une
exception, et l'image en cours de lecture est libérée : pas de fuite mémoire.

## Performances

Les boucles travaillent directement sur le tableau de pixels, ligne par ligne :
les bornes sont vérifiées une fois au début, pas à chaque pixel. Les fichiers TGA
et maison sont lus et écrits par lignes entières plutôt qu'octet par octet, et
les redimensionnements calculent une seule fois ce qui ne dépend que de la colonne.
Les calculs sont faits dans le même ordre qu'avant, donc les images produites sont
identiques à l'octet près.

Mesuré sur une image de 4000 × 3000 (g++ -O2) :

| Opération                  | Avant   | Après  |
|----------------------------|---------|--------|
| Redimensionnement bilinéaire, couleur | 214 ms | 137 ms |
| Redimensionnement bilinéaire, gris    | 49 ms  | 24 ms  |
| Écriture TGA compressé RLE  | 221 ms | 105 ms |
| Écriture TGA brut           | 184 ms | 96 ms  |
| Lecture TGA                 | 112 ms | 33 ms  |
| Remplissage                 | 6,7 ms | 3,1 ms |

## Compiler

Aucune dépendance, un compilateur C++17 suffit :

```bash
g++ -std=c++17 -Wall -c src/Image.cpp
```

Il n'y a pas de programme de démonstration : la bibliothèque s'utilise en
incluant `src/Image.hpp`. Deux images d'essai sont fournies dans `samples/`.

## Organisation du dépôt

```
src/Image.hpp        classes Color, Image<T>, GrayImage, ColorImage
src/Image.cpp        lecture / écriture des formats, conversions
samples/             images d'essai (chat.pgm, chat_petit.ppm)
```
