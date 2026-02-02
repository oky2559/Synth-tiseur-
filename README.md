# Synthétiseur Audio en C

Synthétiseur audio modulaire en C. Il génère de la musique à partir de fichiers de partition textuels, affiche l’onde en temps réel et exporte en `.wav`.

Deux modes d’utilisation :
- **GUI (GTK3)** pour la visualisation et le contrôle interactif
- **CLI** pour une génération rapide

---

## 🚀 Fonctionnalités

### 🎹 Synthèse audio
- **Synthèse additive** avec polyphonie (accords et notes superposées)
- **4 formes d’onde** :
  - `0` : Sinusoïdale
  - `1` : Carrée
  - `2` : Dents de scie
  - `3` : Triangulaire
- **Enveloppe ADSR automatique** pour éviter les clics et adoucir le son

### 🖥️ Interface graphique (GTK3)
- Visualisation de la forme d’onde (Cairo)
- Boutons **Lire / Pause / Stop**, curseur de **Volume**
- Barre de progression (ligne rouge)
- Export automatique de `output.wav` dans le dossier **Téléchargements**

### ⚡ Mode CLI
- Génération de WAV sans interface graphique

---

## 🛠 Pré-requis

- **Compilateur C** : GCC ou Clang
- **Build** : Meson + Ninja
- **GUI** : GTK+ 3.0 (développement)
- **Audio** : SoX (`play`) ou ALSA (`aplay`)

### Installation

**Ubuntu / Debian**
```bash
sudo apt update
sudo apt install build-essential meson ninja-build libgtk-3-dev sox
```

**macOS (Homebrew)**
```bash
brew install meson ninja gtk+3 sox
```

---

## 📦 Compilation

```bash
meson setup builddir
meson compile -C builddir
```

---

## ▶️ Utilisation

### GUI
```bash
./builddir/app/base_project_app
```

### CLI
```bash
./builddir/app/base_project_app --cli data/game.txt
```

---

## 🎵 Format des fichiers de partition (.txt)

Chaque ligne contient une note au format :

```
[TYPE] [DEBUT] [DUREE] [FREQUENCE]
```

- **TYPE** : 0=Sinus, 1=Carré, 2=Scie, 3=Triangle
- **DEBUT** : temps de départ (secondes)
- **DUREE** : duréex (secondes)
- **FREQUENCE** : fréquence (Hz)

**Exemple :**
```
0 0.0 0.5 440    # Note 1 : Sinus, La4
2 0.5 0.5 880    # Note 2 : Scie, La5
0 0.0 1.0 220    # Note 3 : Basse (polyphonie)
```

---

## 📂 Architecture du projet

- src/ : cœur du synthétiseur (calcul audio)
- include/ : headers
- app/ : application GUI/CLI
- data/ : exemples de partitions
- tests/ : tests unitaires
- meson.build : configuration de build# Synthétiseur Audio en C
