#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>




class Color {
public:
    uint8_t r, g, b;
    inline Color(uint8_t _r = 0, uint8_t _g = 0, uint8_t _b = 0)
        : r(_r), g(_g), b(_b) {}
    friend bool  operator==(const Color& c1, const Color& c2);
    friend Color operator*(double alpha, const Color& c);
    friend Color operator+(const Color& c1, const Color& c2);

    void fromCMY(uint8_t c, uint8_t m, uint8_t y);
};


template <typename PixelType>
class Image {
    private:
        const uint16_t width, height;


    protected:
        PixelType *array;

        // nombre de pixels : en size_t AVANT la multiplication, sinon le calcul se fait en int
        // et deborde des 46341 x 46341 pixels (65535 x 65535 depasse 2^31)
        size_t taille() const { return size_t(width) * height; }

    public:

        Image& operator=(const Image& b) = delete;


        Image(uint16_t w, uint16_t h)
        : width(w), height(h), array(nullptr) {
            if (w == 0 || h == 0)
                throw std::invalid_argument("Il ne peux pas y avoire de dimention null");
            array = new PixelType [taille()];
        }


        // l'original a deja ete verifie a sa construction, pas besoin de revoir ses dimensions
        Image(const Image& orig)
        : width(orig.width), height(orig.height), array(new PixelType [orig.taille()]) {
            std::copy(orig.array, orig.array + taille(), array);
        }

        ~Image() {
            delete [] array;
        }

        const uint16_t& getWidth() const { return width; }
        const uint16_t& getHeight() const { return height; }

        PixelType& pixel(uint16_t x, uint16_t y) {
            if (x >= width || y >= height)
                throw std::out_of_range("Accès hors limites");
            return array[size_t(y) * width + x];
        }


         const PixelType& pixel(uint16_t x, uint16_t y) const {
            if (x >= width || y >= height)
                throw std::out_of_range("Accès hors limites");
            return array[size_t(y) * width + x];
        }

        void clear(PixelType defaultValue = PixelType()) {
            std::fill(array, array + taille(), defaultValue);
        }


        // les bornes sont verifiees une fois au debut : dans les boucles on ecrit directement
        // dans le tableau, sans repasser par pixel() et sa verification a chaque pixel
        void rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PixelType value) {
            if (w == 0 || h == 0)
                throw std::invalid_argument("il ne peux pas y avoir de dimention null");

            if (x + w > width || y + h > height)
                throw std::out_of_range("rectangle hors de l'image");

            PixelType* haut = array + size_t(y) * width + x;
            PixelType* bas = array + size_t(y + h - 1) * width + x;
            std::fill(haut, haut + w, value);
            std::fill(bas, bas + w, value);

            for(uint16_t k = y; k < y + h; k++) {
                PixelType* ligne = array + size_t(k) * width;
                ligne[x] = value;
                ligne[x + w - 1] = value;
            }
        }


        void fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PixelType value) {
            if (w == 0 || h == 0)
                throw std::invalid_argument("il ne peux pas y avoir de dimention null");

            if (x + w > width || y + h > height)
                throw std::out_of_range("rectangle hors de l'image");

            for(size_t i = y; i < size_t(y) + h; i++) {
                PixelType* debut = array + i * width + x;
                std::fill(debut, debut + w, value);
            }

        }



        Image<PixelType>* simpleScale(uint16_t w, uint16_t h) const {
            if (w == 0 || h == 0)
                throw std::invalid_argument("On ne peut pas diviser par 0");

            Image<PixelType>* img = new Image<PixelType>(w, h);

            const double coefx = double(width) / w;
            const double coefy = double(height) / h;

            // la colonne source ne depend que de xp : calculee une fois pour toutes les lignes
            std::vector<uint16_t> colonne(w);
            for (uint16_t xp = 0; xp < w; xp++)
                colonne[xp] = uint16_t(xp * coefx);

            for (uint16_t yp = 0; yp < h; yp++) {
                const PixelType* source = array + size_t(uint16_t(yp * coefy)) * width;
                PixelType* dest = img->array + size_t(yp) * w;
                for (uint16_t xp = 0; xp < w; xp++)
                    dest[xp] = source[colonne[xp]];
            }
            return img;
        }


        Image<PixelType>* bilinearScale(uint16_t w, uint16_t h) const {
            if (w == 0 || h == 0)
                throw std::invalid_argument("On ne peut pas diviser par 0");

            Image<PixelType>* img = new Image<PixelType>(w, h);

            const double coefx = double(width) / w;
            const double coefy = double(height) / h;

            // tout ce qui ne depend que de la colonne est calcule une seule fois
            std::vector<uint16_t> xi(w), xi1(w);
            std::vector<double> lambda(w);
            for (uint16_t xp = 0; xp < w; ++xp) {
                const double x = xp * coefx;
                xi[xp] = uint16_t(x);
                lambda[xp] = x - xi[xp];
                xi1[xp] = (xi[xp] + 1 < width) ? xi[xp] + 1 : xi[xp];
            }

            for (uint16_t yp = 0; yp < h; ++yp) {
                const double y = yp * coefy;
                const uint16_t yi = uint16_t(y);
                const double mu = y - yi;
                const uint16_t yi1 = (yi + 1 < height) ? yi + 1 : yi;
                const PixelType* l0 = array + size_t(yi) * width;      // ligne du dessus
                const PixelType* l1 = array + size_t(yi1) * width;     // ligne du dessous
                PixelType* dest = img->array + size_t(yp) * w;

                for (uint16_t xp = 0; xp < w; ++xp) {
                    const double l = lambda[xp];
                    // meme calcul, dans le meme ordre, que la version pixel par pixel : meme resultat
                    dest[xp] = PixelType(
                          (1 - l) * (1 - mu) * l0[xi[xp]] + l * (1 - mu) * l0[xi1[xp]]
                        + (1 - l) * mu * l1[xi[xp]]       + l * mu * l1[xi1[xp]]
                    );
                }
            }
            return img;
        }

};






class GrayImage : public Image<uint8_t> {


public:
    GrayImage& operator=(const GrayImage& b) = delete;
    GrayImage(uint16_t w, uint16_t h);
    GrayImage(const GrayImage& orig);
    GrayImage(const Image<uint8_t>& orig);
    ~GrayImage();


    void writePGM(std::ostream& os) const;

    static GrayImage* readPGM(std::istream& is);

    void writeTGA(std::ostream& os) const;
    static GrayImage* readTGA(std::istream& is);


    GrayImage* simpleScale(uint16_t w, uint16_t h) const;
    GrayImage* bilinearScale(uint16_t w, uint16_t h) const;
};



class ColorImage : public Image<Color> {
public:
    ColorImage& operator=(const ColorImage&) = delete;
    ColorImage(uint16_t w, uint16_t h);
    ColorImage(const ColorImage& orig);
    ColorImage(const Image<Color>& orig);
    ~ColorImage();


    void writePPM(std::ostream& os) const;
    static ColorImage* readPPM(std::istream& is);

    void writeTGA(std::ostream& os, bool rle = true) const;
    static ColorImage* readTGA(std::istream& is);

    ColorImage* simpleScale(uint16_t w, uint16_t h) const;
    ColorImage* bilinearScale(uint16_t w, uint16_t h) const;

    static ColorImage * readMaison(std::istream& is);

    ColorImage * anaglyphe();

};

#endif
