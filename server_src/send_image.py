import sqlite3
import datetime
import string
import random
import time

import json
from PIL import Image as Image

images_db = "/var/jail/home/team43/laser_comms/images.db"

db_exists = False

def create_database():
    conn = sqlite3.connect(images_db)  # connect to that database (will create if it doesn't already exist)
    c = conn.cursor()  # move cursor into database (allows us to execute commands)
    c.execute(f'''CREATE TABLE IF NOT EXISTS images (image_name text, pixel_values text, timing timestamp);''') # run a CREATE TABLE command
    conn.commit() # commit commands (VERY IMPORTANT!!)
    conn.close() # close connection to database
    db_exists = True

create_database()  #call the function!

def insert_into_database(image_name, pixel_values):
    conn = sqlite3.connect(images_db)
    c = conn.cursor()
    c.execute('''INSERT into images VALUES (?,?,?);''',(image_name, pixel_values,datetime.datetime.now()))
    conn.commit()
    conn.close()


def lookup_database():
    conn = sqlite3.connect(images_db)
    c = conn.cursor()
    things = c.execute('''SELECT * FROM images ORDER BY timing DESC LIMIT 1;''').fetchall()
    for row in things:
        print(row)
    conn.commit()
    conn.close()
    return things

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
    # new image and continuring sending chunks
    if request["method"]=="GET" and 'new_image' in request['values'] :
        create_database()
        image_ = request['values']['new_image']
        image = load_color_image(f'/var/jail/home/team43/laser_comms/{image_}')
        data = [get_pixel(image, x, y) for x in range(image['width']) for y in range(image['height'])]
        converted_data = rgb565_convert(data)
        data_as_json_str = json.dumps(converted_data)
        insert_into_database(image_,data_as_json_str)
        send_data = converted_data[0:160]
        response = {'pixels': send_data}
        return json.dumps(response)
    elif request["method"]=="GET" and 'chunk' in request['values']:
        chunk = int(request['values']['chunk']) #slice [chunk:chunk+160]
        data = lookup_database()
        image_name, pixels_list_as_str, upload_time = data[0][0], data[0][1], data[0][2]
        pixels_list = json.loads(pixels_list_as_str)
        send_data = pixels_list[(chunk*160):(chunk*160)+160]
        response = {'pixels': send_data}
        return json.dumps(response)








        # except Exception as e:
        #     # return e here or use your own custom return message for error catch
        #     #be careful just copy-pasting the try except as it stands since it will catch *all* Exceptions not just ones related to number conversion.
        #     return "Error: image is missing"
    
    
    

        

