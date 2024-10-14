#include <stm32f031x6.h>
#include "display.h"

typedef struct Sprite
{
    int width, height;
    uint16_t *Image;
} sprite;

typedef struct Image
{
    int x, y;
    sprite *sprite;
    int hOrientation, vOrientation;
    int hmoved, vmoved;

} image;

void buildImage(image *img, int x, int y, sprite *sprite, int hOrientation, int vOrientation)
{
    img->x = x;
    img->y = y;
    
    img->sprite = sprite;
    img->hOrientation = hOrientation;
    img->vOrientation = vOrientation;
    img->hmoved = img->vmoved = 0;
}

sprite buildSprite(sprite *sp, int width, int height, uint16_t *Image)
{   
    sp->height = height;
    sp->width = width;
    sp->Image = Image;


}

void putImage_struct(image *img)
{
    putImage(img->x, img->y, img->sprite->width, img->sprite->height, img->sprite->Image, img->hOrientation, img->vOrientation);
}
