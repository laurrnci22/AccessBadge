# Access Badge 🛡️

**Access Badge** est un système complet de gestion d'accès et d'authentification via la technologie NFC. Ce projet permet de déverrouiller une application Web en scannant un badge utilisateur sur un lecteur physique personnalisé.

## 🏗️ Architecture du projet

Le projet se compose de trois modules interconnectés :

* **Microcontrôleur (C / AVR) :** Code bas niveau gérant le lecteur NFC (PN532), l'écran OLED et la communication USB (via LUFA).
* **API Backend (Python) :** Service agissant comme une passerelle entre le périphérique USB et l'application Web.
* **Frontend (React) :** Interface utilisateur de démonstration pour l'authentification.

---

## 🚀 Installation et Démarrage

Suivez ces étapes dans l'ordre pour lancer le projet complet.

### 1. Flashage du Microcontrôleur 🔌

Cette étape charge le firmware sur la carte ATmega32u4.

1.  Connectez votre carte en USB à l'ordinateur.
2.  Ouvrez un terminal et naviguez vers le dossier du firmware :
    ```bash
    cd lufa-LUFA-210130-NSI/I2L/Minimal/
    ```
3.  Compilez le code :
    ```bash
    make
    ```
4.  Mettez le microcontrôleur en **mode Bootloader** (appuyez sur le bouton RESET de la carte).
5.  Flashez le firmware :
    ```bash
    make flash
    ```
   > *Le microcontrôleur devrait redémarrer automatiquement après le flash.*

### 2. Lancement de l'API Python 🐍

Ce service doit rester ouvert pour faire le lien entre l'USB et le Web.

1.  Ouvrez un **nouveau terminal** et allez dans le dossier de l'API :
    ```bash
    cd apk
    ```
2.  Installez les dépendances (si ce n'est pas déjà fait) 
3.  Lancez le script (les droits `sudo` sont requis pour l'accès USB) :
    ```bash
    sudo python3 usb_service.py
    ```

### 3. Lancement de l'Application React ⚛️

1.  Ouvrez un **troisième terminal** et allez dans le dossier de l'application :
    ```bash
    cd nfc-app
    ```
2.  Installez les paquets Node.js :
    ```bash
    npm install
    ```
3.  Démarrez le serveur de développement :
    ```bash
    npm run dev
    ```
4.  Ouvrez votre navigateur à l'adresse indiquée (ex: `http://localhost:5173`).

---

## 📖 Utilisation

1.  Assurez-vous que les 3 terminaux tournent sans erreur.
2.  **Sur le boîtier :** Approchez un badge NFC du lecteur.
3.  **Sur l'écran OLED :** Les informations du badge (Nom, Prénom) s'affichent.
4.  **Sur l'Application Web :** L'utilisateur est authentifié automatiquement et les infos du badge apparaissent à l'écran.