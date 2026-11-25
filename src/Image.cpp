#include "Image.hpp"


//color ---

bool operator==(const Color& c1, const Color& c2) {
    return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}


Color operator*(double alpha, const Color& c) {
    return Color(uint8_t(alpha * c.r + 0.5), uint8_t(alpha * c.g + 0.5), uint8_t(alpha * c.b + 0.5));
}

Color operator+(const Color& c1, const Color& c2) {
    int r = c1.r + c2.r, g = c1.g + c2.g, b = c1.b + c2.b;      // a la base je voulais mettre des %255 mais si ca dépasse la je met juste la maximum 255
    return Color(uint8_t(r > 255 ? 255 : r),
                 uint8_t(g > 255 ? 255 : g),
                 uint8_t(b > 255 ? 255 : b));
}

void Color::fromCMY(uint8_t c, uint8_t m, uint8_t y) {
    r = 255 - c;
    g = 255 - m;
    b = 255 - y;
}

// Fonction général

void skip_line(std::istream& is) {
    char c = ' ';
    while (is.get(c) && c!='\n') {}
}

void skip_comments(std::istream& is) {
    char c = ' ';
    while (is.get(c) && c == '#') {
        skip_line(is);
    }
    is.putback(c);
}



// GrayImage-----

GrayImage::GrayImage(uint16_t w, uint16_t h)
: Image<uint8_t>(w, h) {}

GrayImage::GrayImage(const GrayImage& o)
: Image<uint8_t>(o) {}

GrayImage::GrayImage(const Image<uint8_t>& o)
: Image<uint8_t>(o) {}

GrayImage::~GrayImage() {}          //pas de delet car Image le fait deja 

void GrayImage::writePGM(std::ostream& os) const {
    os << "P5\n"
    << getWidth() << " " << getHeight() << "\n"
    << "#bonjour\n"
    << "255\n";
    os.write((const char*) array, getWidth()*getHeight());
}



GrayImage* GrayImage::readPGM(std::istream& is) {
    std::string magic_nbr = "";
    getline(is, magic_nbr);
    if (magic_nbr != "P5" && magic_nbr != "P2")         // P5 pour ASCII+binaire et P2 pour ASCII pur
        throw std::runtime_error("pas sur PGM valide");
    skip_comments(is);
    uint16_t width, height;
    if (!(is >> width >> height))
        throw std::runtime_error("impossible de lire");
    skip_line(is);
    skip_comments(is);
    int ligne = 0;
    is >> ligne;
    if (ligne != 255)
        throw std::runtime_error("Impossible de lire");
    char c = ' ';
    is.get(c);

    GrayImage* img = new GrayImage(width, height);

    if (magic_nbr == "P5") {

        is.read((char*) img->array, size_t(width) * height);

        if (!is) {
            delete img;
            throw std::runtime_error("fichier PGM tronque");
        }

    } else {                                   // P2 les pixels sont du texte
        for (size_t t = 0; t < size_t(width) * height; ++t) {
            int v;
            is >> v;
            if (!is || v < 0 || v > 255) { 
                delete img;
                throw std::runtime_error("valeur de pixel invalide en P2");
            }
            img->array[t] = uint8_t(v);
        }
    }

    return img;
}




GrayImage* GrayImage::simpleScale(uint16_t w, uint16_t h) const {

    Image<uint8_t>* temp = Image<uint8_t>::simpleScale(w, h);
    GrayImage* res = new GrayImage(*temp); 
    delete temp;
    return res;
}

GrayImage* GrayImage::bilinearScale(uint16_t w, uint16_t h) const {
    Image<uint8_t>* temp = Image<uint8_t>::bilinearScale(w, h);
    GrayImage* res = new GrayImage(*temp);
    delete temp;
    return res;
}


void GrayImage::writeTGA(std::ostream& os) const {
    uint8_t entete[18] = {0};
    entete[2] = 3;          // 2 = en couleur et 3 représente le niveau de gris 
    entete[12] = getWidth() % 256;
    entete[13] = getWidth() / 256;
    entete[14] = getHeight() % 256;
    entete[15] = getHeight() / 256;
    entete[16] = 8;     //8 bits par pixel pour le niveau de gris 
    entete[17] = 0x20;
    // les autres entete sont = 0
    os.write((char*) entete, 18);

    os.write((const char*) array, size_t(getWidth()) * getHeight()); // 1 octet par pixel = ca écrit le tab d'un bloc

}


GrayImage* GrayImage::readTGA(std::istream& is) {

    uint8_t entete[18]; // dans la doc on nous dis que l'en tete fais toujour 18 octes
    is.read((char*) entete, 18);


    if (entete[2] != 3) // le type d'image 
        throw std::runtime_error("Problem avec le type, (il peux s'agire d'une image en couleur ou autre)");
    if (entete[16] != 8)     // Image Pixel Size dans la cousigne on nous demande supporter seulement les images de 8 bits
        throw std::runtime_error("problem avec les bits, (il peux s'agire d'une image en couleur ou autre)");

    const uint16_t w = entete[13] * 256 + entete[12];
    const uint16_t h = entete[15] * 256 + entete[14];
    if(w == 0)
        throw std::runtime_error("dimensions invalides w = 0");
    if(h == 0)
        throw std::runtime_error("dimensions invalides h = 0");
    

    is.ignore(entete[0]);          // saute le champ ID, de longueur variable

    const bool basGauche = !(entete[17] & 0x20);   // bit 5 a 0 = origine en bas a gauche

    GrayImage* img = new GrayImage(w, h);

    for (uint16_t y = 0; y < h; ++y) {
        const uint16_t ligne = basGauche ? h - 1 - y : y;   // la 1re ligne du fichier va en bas
        is.read((char*) &img->pixel(0, ligne), w);          // 1 octet par pixel une ligne = w octets 
        if (!is) {
            delete img;                                     // pas de fuite memoire
            throw std::runtime_error("fichier TGA tronque");
        }
    }
    return img;
}









//Colored image ------------------------------

ColorImage::ColorImage(uint16_t w, uint16_t h)
: Image<Color>(w, h) {}

ColorImage::ColorImage(const ColorImage& o)
: Image<Color>(o) {}

ColorImage::ColorImage(const Image<Color>& o)
: Image<Color>(o) {}


ColorImage::~ColorImage() {}        // pareil que pour Gray

void ColorImage::writePPM(std::ostream& os) const {
    os << "P6\n"
    << getWidth() << " " << getHeight() << "\n"
    << "#bonjour\n"
    << "255\n";
    os.write((const char*) array, getWidth()*getHeight()*3);
}

ColorImage* ColorImage::readPPM(std::istream& is) {
    std::string magic_nbr = "";
    getline(is, magic_nbr);
    if (magic_nbr != "P6" && magic_nbr != "P3")         //P6 pour ASCII+binaire et P3 pour ASCII pur, meme principe que grayimage
        throw std::runtime_error("pas sur PGM valide");
    skip_comments(is);
    uint16_t width, height;
    if (!(is >> width >> height))
        throw std::runtime_error("impossible de lire");
    skip_line(is);
    skip_comments(is);
    int ligne = 0;
    is >> ligne;
    if (ligne != 255)
        throw std::runtime_error("Impossible de lire");
    char c = ' ';
    is.get(c);
    ColorImage* img = new ColorImage(width, height);
    if (magic_nbr == "P6") {

        is.read((char*) img->array, size_t(width) * height * 3);

        if (!is) {
            delete img;
            throw std::runtime_error("fichier PPM tronque");
        }

    } else {                                   // P3 = trois nombres par pixel
        for (size_t t = 0; t < size_t(width) * height; ++t) {
            int r, g, b;
            is >> r >> g >> b;
            if (!is || r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
                delete img;
                throw std::runtime_error("valeur de pixel invalide en P3");
            }
            img->array[t] = Color(uint8_t(r), uint8_t(g), uint8_t(b));
        }
    }

    return img;
}


ColorImage* ColorImage::simpleScale(uint16_t w, uint16_t h) const {

    Image<Color>* temp = Image<Color>::simpleScale(w, h);
    ColorImage* res = new ColorImage(*temp);
    delete temp;
    return res;
}

ColorImage* ColorImage::bilinearScale(uint16_t w, uint16_t h) const {
    Image<Color>* temp = Image<Color>::bilinearScale(w, h);
    ColorImage* res = new ColorImage(*temp);
    delete temp;
    return res;
}

void ColorImage::writeTGA(std::ostream& os, bool rle) const {
    uint8_t entete[18] = {0};
    entete[2]  = rle ? 10 : 2;      // type 10 si on compresse en RLE, sinon type 2
    entete[12] = getWidth()  % 256;      // reste de la division 8 bits de droite
    entete[13] = getWidth()  / 256;      // quotient = les 8 bits de gauche
    entete[14] = getHeight() % 256;      // pareil
    entete[15] = getHeight() / 256;
    entete[16] = 24;                // 24 bit par pixel  8 r, 8g, 8b
    entete[17] = 0x20;              // l'octet 17 indique si l'image commenec en haut ou en bas 
    os.write((char*) entete, 18);   //// envoie les 18 octets d'un coup

    if (!rle) {                         // si c'est type 2 
        for (size_t t = 0; t < size_t(getWidth()) * getHeight(); ++t) {   // ici ca parcoure le tableau a plat
            os.put(array[t].b);     // bgr car en TGA veut dans l'ordre bleu vert rouge 
            os.put(array[t].g);
            os.put(array[t].r);
        }
        return;
    }

    // si type 10 je fais le RLE, ligne par ligne
    // au lieu de maqué 5 pixel rouge, je vais venir faire 5*couleur rouge (comme vu dans la lecon)
    for (uint16_t y = 0; y < getHeight(); ++y) {
        uint16_t x = 0;     //indice de ou j'en suis sur la ligne
        while (x < getWidth()) {
            uint16_t n = 1; // il y a au minimu le pixel courant donc 1 
            while (x + n < getWidth() && n < 128 && pixel(x + n, y) == pixel(x, y))      // qu'on ne dépasse pas le finde de ligne et que le maquet fais 128 pixel max et si meme couleur
                ++n;

            if (n >= 2) {  // un paquet est repéré au moin deux pixels pareil
                os.put(uint8_t(0x80 | (n - 1)));  // bit 7 a 1 = paquet repete ; les 7 autres bits portent n - 1 car un pixel 0 n'éxiste pas donc la quantité réel de 1 a 128pixels est stoké sous forme 0 a 127
                // donc pour faire simple ce put indiquera donc d'ue part si il doit répété la couleur (1 ou 0, et le nombre de fois qu'il faut le répété la couleur qui vient après 
                os.put(pixel(x, y).b);      // pixel() renvoie un Color& car ColorImage hérite de Image<Color>
                os.put(pixel(x, y).g);
                os.put(pixel(x, y).r);
            } else {                              // paquet brut
                while (x + n < getWidth() && n < 128 && !(pixel(x + n, y) == pixel(x + n - 1, y)))       // contrairement a l'autre while qui cherche des pixels pareil lui note les pixels différents 
                    ++n;
                os.put(uint8_t(n - 1)); // comment pour les paquet la on va venir fais des paquets avec le nombre de pixels diférents en tete et ensuite les couleur diférente,  ca permet de gagner pas mal de place
                for (uint16_t i = 0; i < n; ++i) {
                    os.put(pixel(x + i, y).b);
                    os.put(pixel(x + i, y).g);
                    os.put(pixel(x + i, y).r);
                }
            }
            x += n; // je saute les n pixels que je viens de traiter 
        }
    }
}



ColorImage* ColorImage::readTGA(std::istream& is) {
    uint8_t entete[18];
    is.read((char*) entete, 18);

    const uint8_t type = entete[2];
    if (type != 1 && type != 2)
        throw std::runtime_error("TGA non supporte (type 1 ou 2 seulement)");


    // je reassemble les 2 octets en un nombre 16 bits
    // l'octet de poids fort vaut 256 fois plus que celui de poids faible
    const uint16_t w = entete[13] * 256 + entete[12];
    const uint16_t h = entete[15] * 256 + entete[14];
    if (w == 0 || h == 0)
        throw std::runtime_error("dimensions invalides");

    const uint16_t nb = entete[6] * 256 + entete[5];   //les octets 5-6 disent combien de couleurs contient la palette (en little indian) pour pour le type 1 et une vérif du type 2


    is.ignore(entete[0]);          // je saute le champ ID, avec l'octet 0 qui donne qui donne la longueur de cham de text


    const bool basGauche = !(entete[17] & 0x20);   // comme pour grayimage


    
    // type 2 
    if (type == 2) {
        if (entete[16] != 24)       // j'ai fais que en 24 er pas en 32 ou 16 donc vérif
            throw std::runtime_error("TGA non supporte (seulement 24 bits)");

        if (entete[1] == 1)                          // palette inutile présente je la saute
            is.ignore(size_t(nb) * entete[7] / 8);


        ColorImage* img = new ColorImage(w, h);
        for (uint16_t y = 0; y < h; ++y) {
            const uint16_t ligne = basGauche ? h - 1 - y : y;
            for (uint16_t x = 0; x < w; ++x) {
                uint8_t bgr[3];                         // tableau de 3 (bgr)
                is.read((char*) bgr, 3);
                if (!is) {
                    delete img;                         // le delete avant le throw pour pas avoir de fuite memoire
                    throw std::runtime_error("fichier TGA tronque");
                }
                img->pixel(x, ligne) = Color(bgr[2], bgr[1], bgr[0]);   // Color attend r g b, le TGA stocke b g r
            }
        }
        return img;
    }

    
    
    
    
    // type 1 image palette 
    // pas besoin de faire de vérif car il y a que le type 1 ou 2 donc si c'est pas le deux c'est le 1
    if (entete[16] != 8 || entete[7] != 24) 
        throw std::runtime_error("TGA a palette non supporte (indices 8 bits, palette 24 bits)");
    if (entete[1] != 1)                         // pareil que type 2
        throw std::runtime_error("TGA type 1 sans palette");
    

    

    if (nb == 0 || nb > 256)  // petite vérif 
        throw std::runtime_error("palette invalide");
    

    

    Color palette[256];     //je crée la palette 
    for (uint16_t i = 0; i < nb; ++i) {
        uint8_t bgr[3];
        is.read((char*) bgr, 3);
        palette[i] = Color(bgr[2], bgr[1], bgr[0]);     // BGR du fichier = Color(r, g, b)
    }


    if (!is)
        throw std::runtime_error("palette TGA tronquee");

    ColorImage* img = new ColorImage(w, h);
    for (uint16_t y = 0; y < h; ++y) {
        const uint16_t ligne = basGauche ? h - 1 - y : y;       // meme pricipe que pour type 2 et gray image
        for (uint16_t x = 0; x < w; ++x) {
            uint8_t indice;
            is.read((char*) &indice, 1);
            if (!is || indice >= nb) {
                delete img;
                throw std::runtime_error("fichier TGA tronque ou indice de palette invalide");
            }
            img->pixel(x, ligne) = palette[indice];     // le fichier contient un indice, on le traduit en couleur
        }
    }
    return img;


}



ColorImage * ColorImage::readMaison(std::istream& is) {
    uint8_t entete[9];          // "Maison" + taille du commentaire + largeur + hauteur
    is.read((char*) entete, 9);

    if(!is) {
        throw std::runtime_error("problem avec le type de fichier ");
    }

    if(entete[0] != 'M')
        throw std::runtime_error("problem avec l'entete"); 
    if(entete[1] != 'a')
        throw std::runtime_error("problem avec l'entete"); 
    if(entete[2] != 'i')
        throw std::runtime_error("problem avec l'entete"); 
    if(entete[3] != 's')
        throw std::runtime_error("problem avec l'entete"); 
    if(entete[4] != 'o')
        throw std::runtime_error("problem avec l'entete"); 
    if(entete[5] != 'n')
        throw std::runtime_error("problem avec l'entete"); 

    uint16_t commentaireLen = entete[6];
    uint16_t w = entete[7];
    uint16_t h = entete[8];     // derniere case : le tableau va de 0 a 8

    if (w == 0|| h == 0)
        throw std::invalid_argument("il ne peux pas y avoir de dimention null");

    std::string commentaire(commentaireLen, ' ');       // la place pour tout le commentaire
    is.read(&commentaire[0], commentaireLen);
    if (!is)
        throw std::runtime_error("commentaire maison tronque");
    std::cout << commentaire << std::endl;

    ColorImage* img = new ColorImage(w, h);
    for (uint16_t y = 0; y < h; ++y) {
        for (uint16_t x = 0; x < w; ++x) {
            uint8_t cmy[3];
            is.read((char*) cmy, 3);
            if (!is) {
                delete img; 
                throw std::runtime_error("fichier maison tronque");
            }
            img->pixel(x, y).fromCMY(cmy[0], cmy[1], cmy[2]);
        }
    }
    return img;
}




ColorImage * ColorImage::anaglyphe() {
    uint16_t w = getWidth() / 2;        // l'image contient les deux vues cote a cote
    uint16_t h = getHeight();

    ColorImage* img = new ColorImage(w, h);

    for(uint16_t y = 0; y < h; ++y) {
        for(uint16_t x = 0; x < w; ++x) {
            // rouge de la vue de gauche, vert et bleu de la vue de droite
            img->pixel(x, y) = Color(pixel(x, y).r, pixel(x + w, y).g, pixel(x + w, y).b);
        }
    }
    return img;
}

