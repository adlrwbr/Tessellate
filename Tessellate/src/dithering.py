import sys, math
from PIL import Image
from tqdm import tqdm
from vector import Vector

def main(argv):
    assert len(sys.argv) == 2, "Usage: dithering.py <imagefile>"
    imagePath = sys.argv[1]
    img = Image.open(imagePath)

    # Resize image
    print(f"Resizing image {imagePath}")
    img = img.resize((51, 51), Image.ANTIALIAS)

    # dither
    FloydSteinbergDither(img)

    img.save("output.bmp")

def find_closest_palette_color(pixel):
    colors = {
        "RED": Vector(255, 0, 0),
        "ORANGE": Vector(255, 165, 0),
        "YELLOW": Vector(255, 255, 0),
        "GREEN": Vector(0, 255, 0),
        "BLUE": Vector(0, 0, 255),
        "WHITE": Vector(255, 255, 255),
    }
    shortestD = 1000
    closest = "ORANGE"
    for c in colors:
        distance = math.sqrt((colors[c][0] - pixel[0]) ** 2 + (colors[c][1] - pixel[1]) ** 2 + (colors[c][2] - pixel[2]) ** 2)
        if distance < shortestD:
            shortestD = distance
            closest = c
    # print(f"closest: {closest}, distance to closest: {shortestD}")
    return colors[closest]

def FloydSteinbergDither(img):
    # get dimensions
    width, height = img.size

    # load all pixels from image
    pixel_map = img.load()

    # dithering algorithm
    for y in tqdm(range(height)):
        for x in range(width):
            # implementation of Floyd-Steinberg dithering algorithm
            oldpixel = Vector(pixel_map[x, y][0], pixel_map[x, y][1], pixel_map[x, y][2])
            newpixel = find_closest_palette_color(oldpixel)
            pixel_map[x, y] = (newpixel[0], newpixel[1], newpixel[2])
            quant_error = oldpixel - newpixel



            if (x + 1 < width):
                pix_E = pixel_map[x + 1, y    ]
                newpix_E = Vector(pix_E[0], pix_E[1], pix_E[2]) + quant_error * 7 / 16
                pixel_map[x + 1, y    ] = (int(newpix_E[0]), int(newpix_E[1]), int(newpix_E[2]))
            if (y + 1 < height):
                if (x - 1 >= 0):
                    pix_SW = pixel_map[x - 1, y + 1]
                    newpix_SW = Vector(pix_SW[0], pix_SW[1], pix_SW[2]) + quant_error * 3 / 16
                    pixel_map[x - 1, y + 1] = (int(newpix_SW[0]), int(newpix_SW[1]), int(newpix_SW[2]))
                pix_S = pixel_map[x    , y + 1]
                newpix_S = Vector(pix_S[0], pix_S[1], pix_S[2]) + quant_error * 5 / 16
                pixel_map[x    , y + 1] = (int(newpix_S[0]), int(newpix_S[1]), int(newpix_S[2]))
                if (x + 1 < width):
                    pix_SE = pixel_map[x + 1, y + 1]
                    newpix_SE = Vector(pix_SE[0], pix_SE[1], pix_SE[2]) + quant_error * 1 / 16
                    pixel_map[x + 1, y + 1] = (int(newpix_SE[0]), int(newpix_SE[1]), int(newpix_SE[2]))



if __name__ == '__main__':
    main(sys.argv[1:])
    # input("press enter to continue")
