import requests
from scipy.io import wavfile
from pydub import AudioSegment
import sqlite3
import datetime
import numpy as np
import time
import json

songs_db = "/var/jail/home/team43/laser_comms/songs.db"
class NpEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.uint8):
            return int(obj)
        elif isinstance(obj, np.floating):
            return float(obj)
        elif isinstance(obj, np.ndarray):
            return obj.tolist()
        else:
            return super(NpEncoder, self).default(obj)

def create_database():
    conn = sqlite3.connect(songs_db)  # connect to that database (will create if it doesn't already exist)
    c = conn.cursor()  # move cursor into database (allows us to execute commands)
    c.execute(
        f'''CREATE TABLE IF NOT EXISTS songs (song_name text, left_wave_values text, right_wave_values text,  timing timestamp);''')  # run a CR>    conn.commit()  # commit commands (VERY IMPORTANT!!)
    conn.close()  # close connection to database


# create_database()  #call the function!

def insert_into_database(song_name, left_values, right_values):
    conn = sqlite3.connect(songs_db)
    c = conn.cursor()
    c.execute('''INSERT into songs VALUES (?,?,?,?);''', (song_name, left_values, right_values,  datetime.datetime.now()))
    conn.commit()
    conn.close()

def lookup_database():
    conn = sqlite3.connect(songs_db)
    c = conn.cursor()
    things = c.execute('''SELECT * FROM songs ORDER BY timing DESC LIMIT 1;''').fetchall()
    for row in things:
        print(row)
    conn.commit()
    conn.close()
    return things


def download_from_url(url, name):
    doc = requests.get(url)
    with open(name, 'wb') as f:
        f.write(doc.content)

def convert_mp3_wav(src_name, dst_name):
    #sound = AudioSegment.from_mp3(src_name)
    #sound = sound.set_frame_rate(4000)
    #sound.export(dst_name, format="wav")

    samplerate, raw_data = wavfile.read(f'{dst_name}')
    wavfile.write(f'{dst_name}', 4000, raw_data.astype(np.uint8))
    samplerate, data = wavfile.read(f'{dst_name}')
    left_data = [data[i][0] for i in range(len(data))]
    right_data = [data[i][1] for i in range(len(data))]
    # print(min(data[i][0] for i in range(len(data))))
    # print(len(right_data))
    # print(samplerate)
    return left_data, right_data

def get_song_napster():
    res = requests.get(
        "https://api.napster.com/v2.1/tracks/top?apikey=ZTk2YjY4MjMtMDAzYy00MTg4LWE2MjYtZDIzNjJmMmM0YTdm")
    res_json = res.json()
    track = res_json['tracks'][0]
    track_name = track['name']
    track_download_url = track['previewURL']
    print(track)
    print(track_name)
    # print(track_download_url)
    # print(res.json())
    return track_name, track_download_url



def request_handler(request):
    # new image and continuring sending chunks
    if request["method"] == "GET" and 'new_song' in request['values'] :
        create_database()
        #track_name, mp3_download_url = get_song_napster()
        #download_from_url(mp3_download_url,f'/var/jail/home/team43/laser_comms/{track_name}.mp3' ) # f'{t_name}.mp3'
        left_data, right_data = convert_mp3_wav(f'/var/jail/home/team43/laser_comms/Small_Talk.mp3', f'/var/jail/home/team43/laser_comms/Small_T>        l_data_as_json_str = json.dumps(left_data,cls=NpEncoder )
        r_data_as_json = json.dumps(right_data, cls=NpEncoder)
        insert_into_database("Small_Talk",l_data_as_json_str, r_data_as_json)
        send_data = right_data[0:200]
        #send_data = json.dumps((right_data[0:200]),cls=NpEncoder)[0:-1] # change!!!!!!!
        return send_data
        return ret_list
        response = {'right_vals': send_data}
        return json.dumps(response, cls=NpEncoder)
    elif request["method"]=="GET" and 'chunk' in request['values']:
        chunk = int(request['values']['chunk']) #slice [chunk:chunk+160]
        data = lookup_database()
        image_name, left_vals, right_vals,  upload_time = data[0][0], data[0][1], data[0][2], data[0][3]
        right_list = json.loads(right_vals)
        send_data = right_list[(chunk*200):(chunk*200)+200]
        #send_data = json.dumps(send_data,cls=NpEncoder)[0:-1]
        return send_data
        response = {'right_vals': send_data}
        return json.dumps(response)