import json
from PIL import Image as Image


def load_color_image(filename):
    """
    Loads a color image from the given file and returns a dictionary
    representing that image.

    Invoked as, for example:
       i = load_color_image('test_images/cat.png')
    """
    with open(filename, 'rb') as img_handle:
        img = Image.open(img_handle)
        img = img.convert('RGB')  # in case we were given a greyscale image
        img_data = img.getdata()
        pixels = list(img_data)
        w, h = img.size
        return {'height': h, 'width': w, 'pixels': pixels}
def get_pixel(image, x, y):
    #compute real index of pixel and return value
    ix = y*image['width'] + x
    return image['pixels'][ix]

# image = load_color_image('tft_test_image.png')
# data = [get_pixel(image, x, y) for x in range(image['width']) for y in range(image['height'])
def rgb565_convert(data):
    data565 = []
    for (r, g, b) in data:
        data565.append("0x%0.4X" % ((int(r / 255 * 31) << 11) | (int(g / 255 * 63) << 5) | (int(b / 255 * 31))))
    return data565

def request_handler(request):
    if request["method"]=="GET":
        try:
            image_ = request['values']['image']
        except Exception as e:
            # return e here or use your own custom return message for error catch
            #be careful just copy-pasting the try except as it stands since it will catch *all* Exceptions not just ones related to number conversion.
            return "Error: image is missing"
    image = load_color_image(f'/var/jail/home/team43/laser_comms/{image_}')
    data = [get_pixel(image, x, y) for x in range(image['width']) for y in range(image['height'])]
    converted_data = rgb565_convert(data)[0:200]
    response = {'pixels': converted_data}
    return json.dumps(response)
    

        
