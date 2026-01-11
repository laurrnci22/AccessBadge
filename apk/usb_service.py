import usb1
import time
from flask import Flask, jsonify, request
from flask_cors import CORS
import threading
import sys

app = Flask(__name__)
CORS(app)

VENDOR_ID = 0x4242
PRODUCT_ID = 0x0001
EP_IN = 0x81
EP_OUT = 0x02
EP_SIZE = 64
TIMEOUT = 1000

current_card_info = None
usb_handle = None
usb_context = None

def parse_card_data(data):
    if len(data) < 56:
        print(f"Données trop courtes: {len(data)} octets")
        return None
    if data[0] != ord('R'):
        print(f"Type incorrect: {chr(data[0]) if data[0] < 127 else data[0]}")
        return None

    uid_bytes = [b for b in data[1:8] if b != 0]
    uid = ''.join(f'{b:02X}' for b in uid_bytes)

    nom = bytes(data[8:24]).decode('ascii', errors='ignore').strip('\x00 ')
    prenom = bytes(data[24:40]).decode('ascii', errors='ignore').strip('\x00 ')
    password = bytes(data[40:56]).decode('ascii', errors='ignore').strip('\x00 ')

    return {'uid': uid, 'nom': nom, 'prenom': prenom, 'password': password}


def write_to_badge(nom, prenom, password):
    global usb_handle

    if usb_handle is None:
        return {'success': False, 'error': 'Périphérique USB non connecté'}

    try:
        nom_bytes = nom.encode('ascii')[:16]
        prenom_bytes = prenom.encode('ascii')[:16]
        password_bytes = password.encode('ascii')[:16]

        packet = bytearray(64)
        packet[0] = ord('W')
        packet[1] = len(nom_bytes)
        packet[2] = len(prenom_bytes)
        packet[3] = len(password_bytes)
        packet[4:4+len(nom_bytes)] = nom_bytes
        packet[20:20+len(prenom_bytes)] = prenom_bytes
        packet[36:36+len(password_bytes)] = password_bytes

        print("\n=== ENVOI DONNÉES BADGE ===")
        print(f"Nom: {nom} | Prénom: {prenom} | Pass: {password}")
        usb_handle.interruptWrite(EP_OUT, bytes(packet), timeout=5000)
        print("En attente de la réponse...")

        try:
            response = usb_handle.interruptRead(EP_IN, EP_SIZE, timeout=10000)
            if len(response) > 0 and response[0] == ord('W'):
                success = response[1] == 0x01
                print(f"Réponse reçue: {'OK' if success else 'ERREUR'}")
                return {'success': success, 'message': 'Écriture réussie' if success else 'Erreur lors de l\'écriture'}
            return {'success': False, 'error': 'Réponse invalide'}

        except usb1.USBErrorTimeout:
            return {'success': False, 'error': 'Timeout - Pas de réponse du badge'}

    except Exception as e:
        return {'success': False, 'error': str(e)}


def monitor_usb():
    global current_card_info, usb_handle, usb_context

    if usb_context is None or usb_handle is None:
        print("Aucun périphérique USB initialisé, arrêt du monitoring.")
        return

    print("=== Monitoring USB actif ===\n")

    while True:
        try:
            data = usb_handle.interruptRead(EP_IN, EP_SIZE, timeout=TIMEOUT)
            if len(data) > 0:
                print(f"\n[{int(time.time())}] Reçu {len(data)} octets:")
                print("="*60)
                for i in range(0, len(data), 16):
                    hex_part = ' '.join(f'{b:02X}' for b in data[i:i+16])
                    ascii_part = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[i:i+16])
                    print(f"{i:04d}: {hex_part:<48} | {ascii_part}")
                print("="*60)

                card_info = parse_card_data(data)
                if card_info:
                    current_card_info = card_info
                    print("\n=== DONNÉES PARSÉES ===")
                    for k, v in card_info.items():
                        print(f"{k.capitalize():<10}: {v}")
                    print("="*60 + "\n")

        except usb1.USBErrorTimeout:
            pass
        except usb1.USBErrorNoDevice:
            print("\nPériphérique déconnecté.")
            usb_handle = None
            break
        except Exception as e:
            print(f"\nErreur inattendue: {e}")
            import traceback
            traceback.print_exc()
            break

        time.sleep(0.05)

# API
@app.route('/api/nfc/status')
def nfc_status():
    return jsonify({
        'usb_connected': usb_handle is not None,
        'card_detected': current_card_info is not None,
        'info': current_card_info
    })

@app.route('/api/nfc/info')
def get_current_info():
    if current_card_info:
        return jsonify(current_card_info)
    return jsonify({'error': 'Aucune carte scannée'}), 404

@app.route('/api/nfc/register', methods=['POST'])
def write_nfc():
    global usb_handle
    if usb_handle is None:
        return jsonify({'error': 'Périphérique USB non connecté', 'success': False}), 503

    data = request.get_json()
    if not data:
        return jsonify({'error': 'Corps JSON manquant', 'success': False}), 400

    nom = data.get('nom', '').strip()
    prenom = data.get('prenom', '').strip()
    password = data.get('password', '').strip()

    if not nom or not prenom or not password:
        return jsonify({'error': 'nom, prenom et password sont requis', 'success': False}), 400

    if len(nom) > 16 or len(prenom) > 16 or len(password) > 16:
        return jsonify({'error': 'Champs trop longs (max 16 caractères)', 'success': False}), 400

    print(f"\n=== REQUÊTE D'ÉCRITURE BADGE ===\nNom: {nom}, Prénom: {prenom}, Password: {password}")
    result = write_to_badge(nom, prenom, password)
    return jsonify(result), (200 if result['success'] else 500)

# Main
if __name__ == '__main__':
    def run_flask():
        app.run(host='0.0.0.0', port=5000, debug=False, use_reloader=False)

    flask_thread = threading.Thread(target=run_flask, daemon=True)
    flask_thread.start()

    try:
        usb_context = usb1.USBContext()
        usb_handle = usb_context.openByVendorIDAndProductID(VENDOR_ID, PRODUCT_ID)

        if usb_handle is None:
            print("ERREUR : périphérique LUFA non trouvé.")
            print("Vérifie les VID/PID et la connexion du lecteur.")
            sys.exit(1)

        print(f"Périphérique trouvé (VID:0x{VENDOR_ID:04X}, PID:0x{PRODUCT_ID:04X})")

        try:
            if usb_handle.kernelDriverActive(0):
                usb_handle.detachKernelDriver(0)
        except usb1.USBErrorNotFound:
            pass

        usb_handle.claimInterface(0)
        print("Interface 0 revendiquée, lecture/écriture prêtes.\n")

        monitor_usb()

    except usb1.USBErrorAccess:
        print("\nPermission refusée. Lance le script avec sudo :")
        print("    sudo python3 usb_service.py")
    except Exception as e:
        print(f"\nErreur d'initialisation USB : {e}")
        import traceback
        traceback.print_exc()
