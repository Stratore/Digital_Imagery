#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <stdexcept>




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

    public:

        Image& operator=(const Image& b) = delete;


        Image(uint16_t w, uint16_t h) 
        : width(w), height(h), array(nullptr) {
            if (w == 0 || h == 0) 
                throw std::invalid_argument("Il ne peux pas y avoire de dimention null");
            array = new PixelType [size_t(width*height)];
        }


        Image(const Image& orig)
        : width(orig.width), height(orig.height), array(nullptr) {
            if (orig.width == 0 || orig.height == 0) 
                throw std::invalid_argument("Il ne peux pas y avoire de dimention null");
            array = new PixelType [orig.width * orig.height];
            for (size_t t = 0; t < size_t(width * height); t++)
                array[t] = orig.array[t];
        }

        ~Image() {
            delete [] array;
        }

        const uint16_t& getWidth() const { return width; }
        const uint16_t& getHeight() const { return height; }

        PixelType& pixel(uint16_t x, uint16_t y) {
            if (x >= width || y >= height)
                throw std::out_of_range("Accès hors limites");
            return array[y*width+x];
        }


         const PixelType& pixel(uint16_t x, uint16_t y) const {
            if (x >= width || y >= height)
                throw std::out_of_range("Accès hors limites");
            return array[y*width+x]; 
        }

        void clear(PixelType defaultValue = PixelType()) {
            for(size_t i = 0; i < (size_t)getWidth() * (size_t)getHeight(); ++i) 
                array[i] = defaultValue;
        }


        void rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PixelType value) {
            if (w == 0 || h == 0) 
                throw std::invalid_argument("il ne peux pas y avoir de dimention null");

            if (x + w > width || y + h > height)
                throw std::out_of_range("rectangle hors de l'image");

            for(uint16_t i = x; i < x + w; i++) {
                pixel(i, y) = value;
                pixel(i, y + h - 1) = value;
            }

            for(uint16_t k = y; k < y + h; k++) {
                pixel(x, k) = value;
                pixel(x + w -1 , k) = value;
            }
        }


        void fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PixelType value) {
            if (w == 0 || h == 0) 
                throw std::invalid_argument("il ne peux pas y avoir de dimention null");

            if (x + w > width || y + h > height)
                throw std::out_of_range("rectangle hors de l'image");

            for(size_t i = y; i < y + h; i++) {
                for(size_t k = x; k < x + w; k++) 
                    pixel(k, i) = value;
            }

        }



        Image<PixelType>* simpleScale(uint16_t w, uint16_t h) const {
            if (w == 0 || h == 0) 
                throw std::invalid_argument("On ne peut pas diviser par 0");
            
            Image<PixelType>* img = new Image<PixelType>(w, h);

            const double coefx = double(width) / w;
            const double coefy = double(height) / h;

            for (uint16_t yp = 0; yp < h; yp++) {
                for (uint16_t xp = 0; xp < w; xp++) {
                    uint16_t x(xp * coefx);
                    uint16_t y(yp * coefy);
                    img->pixel(xp, yp) = pixel(x, y);
                }
            }
            return img;
        }

        
        Image<PixelType>* bilinearScale(uint16_t w, uint16_t h) const {
            if (w == 0 || h == 0) 
                throw std::invalid_argument("On ne peut pas diviser par 0");

            Image<PixelType>* img = new Image<PixelType>(w, h);
            
            const double coefx = double(width) / w;
            const double coefy = double(height) / h;

            for (uint16_t yp = 0; yp < h; ++yp) {
                for (uint16_t xp = 0; xp < w; ++xp) {
                    const double x = xp * coefx, y = yp * coefy;
                    const uint16_t xi = uint16_t(x), yi = uint16_t(y);
                    const double lambda = x - xi, mu = y - yi;
                    const uint16_t xi1 = (xi + 1 < width)  ? xi + 1 : xi;
                    const uint16_t yi1 = (yi + 1 < height) ? yi + 1 : yi;

                    img->pixel(xp, yp) = PixelType(
                          (1 - lambda) * (1 - mu) * pixel(xi, yi)   + lambda * (1 - mu) * pixel(xi1, yi)
                        + (1 - lambda) * mu * pixel(xi, yi1) + lambda * mu * pixel(xi1, yi1)
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