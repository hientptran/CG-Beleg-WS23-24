_Computergrafik Beleg - Wintersemester 2023/2024_
# Roboter mit Mouse Picking

## Besonderheiten des Programms
- Ansteuerbarer Roboter mittels Benutzerinteraktion:
  - Tastatureingaben
  - Mausklicks
  - Mouse Dragging
- Körperteile per Mausklick auswählbar
- Rotation der Körperteile durch Mouse Dragging

## Verwendete Frameworks und Bibliotheken
- **freeglut** (Fenster- und Eingabeverwaltung)
- **GLFW** (Fenster- und Eingabeverwaltung)
- **GLEW** (Extension Management)
- **glm** (Mathematische Operationen)
- **GLSL** (OpenGL Shading Language – definiert Shader-Programme für die Beleuchtung und Darstellung der Objekte)

## Implementierte Grafikalgorithmen
- Implementierung eines **Szenegraphen** (3D-Objekte hierarchisch 
angeordnet)
- **Transformationen**: Translation, Rotation und Skalierung
- **Shader** für die Beleuchtung und Darstellung der Objekte
- **Shader** für Objektauswahl mittels Mausklick

## Verwendete Tutorials
- opengl-tutorial: Picking with an OpenGL hack, https://www.opengl-tutorial.org/miscellaneous/clicking-onobjects/picking-with-an-opengl-hack/ (Stand 01.02.2024)
