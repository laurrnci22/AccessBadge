#!/usr/bin/env python3
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

def parse_card_data(data):
    """Parse les données reçues depuis l'ATmega"""
    if len(data) < 56:
        print(f"Données trop courtes: {len(data)} octets")
        return None

    if data[0] != ord('R'):
        print(f"Type incorrect: {chr(data[0]) if data[0] < 127 else data[0]}")
        return None

    # Extraire UID (octets 1-7)
    uid_bytes = [b for b in data[1:8] if b != 0]
    uid = ''.join(f'{b:02X}' for b in uid_bytes)

    # Extraire nom (octets 8-23)
    nom_bytes = bytes([b for b in data[8:24]])
    nom = nom_bytes.decode('ascii', errors='ignore').strip('\x00 ')

    # Extraire prénom (octets 24-39)
    prenom_bytes = bytes([b for b in data[24:40]])
    prenom = prenom_bytes.decode('ascii', errors='ignore').strip('\x00 ')

    # Extraire password (octets 40-55)
    password_bytes = bytes([b for b in data[40:56]])
    password = password_bytes.decode('ascii', errors='ignore').strip('\x00 ')

    return {
        'uid': uid,
        'nom': nom,
        'prenom': prenom,
        'password': password
    }

def write_to_badge(nom, prenom, password):
    """Envoie des données vers l'ATmega pour écriture sur badge"""
    global usb_handle

    if usb_handle is None:
        return {'success': False, 'error': 'Périphérique USB non connecté'}

    try:
        # Encoder en bytes ASCII
        nom_bytes = nom.encode('ascii')[:16]
        prenom_bytes = prenom.encode('ascii')[:16]
        password_bytes = password.encode('ascii')[:16]

        # UN SEUL paquet de 64 octets
        packet = bytearray(64)
        packet[0] = ord('W')           # Commande
        packet[1] = len(nom_bytes)     # Longueur nom
        packet[2] = len(prenom_bytes)  # Longueur prénom
        packet[3] = len(password_bytes) # Longueur password

        # Copier les données
        packet[4:4+len(nom_bytes)] = nom_bytes
        packet[20:20+len(prenom_bytes)] = prenom_bytes
        packet[36:36+len(password_bytes)] = password_bytes

        print(f"\n=== ENVOI DONNÉES BADGE ===")
        print(f"Paquet complet ({len(packet)} octets):")
        print(f"  Header: {bytes(packet[:4]).hex()}")
        print(f"  Nom:    {bytes(packet[4:20])}")
        print(f"  Prénom: {bytes(packet[20:36])}")
        print(f"  Pass:   {bytes(packet[36:52])}")

        usb_handle.interruptWrite(EP_OUT, bytes(packet), timeout=5000)

        print("En attente de la réponse...")

        # Recevoir confirmation
        try:
            response = usb_handle.interruptRead(EP_IN, EP_SIZE, timeout=10000)

            if len(response) > 0 and response[0] == ord('W'):
                success = response[1] == 0x01
                print(f"Réponse reçue: {'✅ OK' if success else '❌ ERREUR'}")

                return {
                    'success': success,
                    'message': 'Écriture réussie' if success else 'Erreur lors de l\'écriture'
                }
            else:
                return {'success': False, 'error': 'Réponse invalide'}

        except usb1.USBErrorTimeout:
            return {'success': False, 'error': 'Timeout - Pas de réponse du badge'}

    except Exception as e:
        return {'success': False, 'error': str(e)}

def monitor_usb():
    """Fonction de monitoring USB"""
    global current_card_info, usb_handle

    try:
        with usb1.USBContext() as context:
            handle = context.openByVendorIDAndProductID(VENDOR_ID, PRODUCT_ID)

            if handle is None:
                print("ERREUR: LUFA non trouvé")
                print(f"Recherché: VID:0x{VENDOR_ID:04X} PID:0x{PRODUCT_ID:04X}")
                print("\nPériphériques USB connectés:")
                for device in context.getDeviceList():
                    print(f"  VID:0x{device.getVendorID():04X} PID:0x{device.getProductID():04X}")
                return

            print("=== Monitoring USB (Python avec libusb1) ===")
            print(f"Périphérique trouvé: VID:0x{VENDOR_ID:04X} PID:0x{PRODUCT_ID:04X}")

            try:
                if handle.kernelDriverActive(0):
                    handle.detachKernelDriver(0)
                    print("Driver kernel détaché")
            except usb1.USBErrorNotFound:
                pass

            try:
                handle.claimInterface(0)
                print("Interface 0 claimed")
            except usb1.USBErrorBusy:
                print("ERREUR: Interface déjà utilisée. Fermez les autres programmes USB.")
                return

            usb_handle = handle
            print("✅ API prête pour écriture/lecture")
            print("En attente de badge NFC...\n")

            while True:
                try:
                    data = handle.interruptRead(EP_IN, EP_SIZE, timeout=TIMEOUT)

                    if len(data) > 0:
                        print(f"\n[{int(time.time())}] Reçu {len(data)} octets:")
                        print("="*60)
                        print("DONNÉES BRUTES (HEX):")

                        # Affichage hexadécimal ligne par ligne
                        for i in range(0, len(data), 16):
                            hex_part = ' '.join(f'{b:02X}' for b in data[i:i+16])
                            ascii_part = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[i:i+16])
                            print(f"{i:04d}: {hex_part:<48} | {ascii_part}")

                        print("="*60)

                        # Parser et afficher
                        card_info = parse_card_data(data)
                        if card_info:
                            current_card_info = card_info
                            print("\n=== DONNÉES PARSÉES ===")
                            print(f"UID:      {card_info['uid']}")
                            print(f"Nom:      {card_info['nom']}")
                            print(f"Prénom:   {card_info['prenom']}")
                            print(f"Password: {card_info['password']}")
                            print("="*60 + "\n")

                except usb1.USBErrorTimeout:
                    pass
                except usb1.USBErrorNoDevice:
                    print("\nPériphérique USB déconnecté")
                    usb_handle = None
                    break
                except Exception as e:
                    print(f"\nErreur inattendue: {e}")
                    import traceback
                    traceback.print_exc()
                    break

                time.sleep(0.01)

    except usb1.USBErrorAccess:
        print("\n!!! ERREUR DE PERMISSIONS !!!")
        print("Solution sur macOS:")
        print("  sudo python3 usb_service.py")
        sys.exit(1)
    except Exception as e:
        print(f"\nErreur USB: {e}")
        import traceback
        traceback.print_exc()

# ==================== API ROUTES ====================

@app.route('/api/nfc/status')
def nfc_status():
    """Vérifie l'état du système"""
    return jsonify({
        'usb_connected': usb_handle is not None,
        'card_detected': current_card_info is not None,
        'info': current_card_info
    })

@app.route('/api/nfc/info')
def get_current_info():
    """Récupère les infos de la dernière carte scannée"""
    if current_card_info:
        return jsonify(current_card_info)
    return jsonify({'error': 'Aucune carte scannée'}), 404

@app.route('/api/nfc/write', methods=['POST'])
def write_nfc():
    """
    Écrit des données sur un badge NFC

    Body JSON:
    {
        "nom": "Alice",
        "prenom": "Smith",
        "password": "SecurePass123"
    }
    """
    if usb_handle is None:
        return jsonify({'error': 'Périphérique USB non connecté'}), 503

    data = request.get_json()

    # Validation
    if not data:
        return jsonify({'error': 'Corps JSON manquant'}), 400

    nom = data.get('nom', '')
    prenom = data.get('prenom', '')
    password = data.get('password', '')

    if not nom or not prenom or not password:
        return jsonify({'error': 'nom, prenom et password sont requis'}), 400

    if len(nom) > 16 or len(prenom) > 16 or len(password) > 16:
        return jsonify({'error': 'Champs trop longs (max 16 caractères)'}), 400

    print(f"\n=== REQUÊTE D'ÉCRITURE BADGE ===")
    print(f"Nom: {nom}, Prénom: {prenom}, Password: {password}")

    # Envoyer vers l'ATmega
    result = write_to_badge(nom, prenom, password)

    if result['success']:
        return jsonify(result), 200
    else:
        return jsonify(result), 500

# ==================== MAIN ====================

if __name__ == '__main__':
    def run_flask():
        app.run(host='0.0.0.0', port=5000, debug=False, use_reloader=False)

    flask_thread = threading.Thread(target=run_flask, daemon=True)
    flask_thread.start()

    print("="*60)
    print("API Flask démarrée sur http://localhost:5000")
    print("\nEndpoints disponibles:")
    print("  GET  /api/nfc/status  - État du système")
    print("  GET  /api/nfc/info    - Infos dernière carte")
    print("  POST /api/nfc/write   - Écrire sur badge")
    print("\nExemple curl:")
    print('  curl -X POST http://localhost:5000/api/nfc/write \\')
    print('    -H "Content-Type: application/json" \\')
    print('    -d \'{"nom":"Alice","prenom":"Smith","password":"Pass123"}\'')
    print("="*60 + "\n")

    monitor_usb()
