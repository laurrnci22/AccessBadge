import usb.core
import usb.util
import json
from flask import Flask, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

VENDOR_ID = 0x03EB  # Atmel
PRODUCT_ID = 0x2042  # À ajuster selon votre descripteur USB

current_uid = None

def read_usb():
    global current_uid
    dev = usb.core.find(idVendor=VENDOR_ID, idProduct=PRODUCT_ID)

    if dev is None:
        return None

    dev.set_configuration()
    ep_in = dev[0][(0,0)][0]

    try:
        data = dev.read(ep_in.bEndpointAddress, ep_in.wMaxPacketSize, 100)
        if data[0] == ord('U'):  # UID reçu
            uid_len = data[1]
            uid = ''.join(f'{b:02X}' for b in data[2:2+uid_len])
            current_uid = uid
            return uid
    except usb.core.USBError:
        pass

    return None

@app.route('/api/nfc/status')
def nfc_status():
    uid = read_usb()
    return jsonify({'uid': uid, 'authenticated': uid is not None})

@app.route('/api/nfc/check')
def check_badge():
    # Vérifier si l'UID est autorisé
    authorized_uids = ['04A3B2C1D2E3F4', '...']  # Liste des badges autorisés

    if current_uid in authorized_uids:
        return jsonify({'authorized': True, 'uid': current_uid})

    return jsonify({'authorized': False})

if __name__ == '__main__':
    app.run(port=5000)
