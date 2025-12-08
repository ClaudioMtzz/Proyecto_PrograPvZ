#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

// --- SECCION DE CLASES (LOS PLANOS) ---

// 1. CLASE ZOMBIE
class Zombie {
public:
    sf::Sprite shape;
    float velocidad;

    // Constructor: Se ejecuta cuando nace el zombie
    Zombie(sf::Texture &textura, float x, float y) {
        shape.setTexture(textura);
        shape.setPosition(x, y);
        shape.setScale(0.2f, 0.2f); // Ajustar tamaño
        velocidad = 1.0f;           // Velocidad de movimiento
    }

    // Metodo para moverse (Logica)
    void update() {
        shape.move(-velocidad, 0); // Mover a la izquierda
        
        // Si se sale de la pantalla, que vuelva a aparecer (loop)
        if (shape.getPosition().x < -100) {
            shape.setPosition(850, shape.getPosition().y);
        }
    }

    // Metodo para dibujarse a si mismo
    void draw(sf::RenderWindow &window) {
        window.draw(shape);
    }
};

// 2. CLASE PLANTA
class Planta {
public:
    sf::Sprite shape;

    Planta(sf::Texture &textura, float x, float y) {
        shape.setTexture(textura);
        shape.setPosition(x, y);
        shape.setScale(0.15f, 0.15f);
        // Las plantas no se mueven, asi que no necesitan velocidad
    }

    void draw(sf::RenderWindow &window) {
        window.draw(shape);
    }
};

// --- SECCION PRINCIPAL (EL DIRECTOR DE ORQUESTA) ---

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "PvZ - Fase 5: Clases y Estructura");
    window.setFramerateLimit(60);

    // 1. CARGAR RECURSOS (Importante: Se cargan UNA sola vez aqui)
    sf::Texture texZombie;
    if (!texZombie.loadFromFile("zombie.png")) return -1;

    sf::Texture texPlanta;
    if (!texPlanta.loadFromFile("planta.png")) return -1;

    // 2. CREAR LISTAS DE OBJETOS
    std::vector<Planta> jardin;       // Vector para guardar plantas
    std::vector<Zombie> hordaZombies; // Vector para guardar zombies

    // Vamos a crear un zombie inicial y meterlo al vector
    hordaZombies.push_back(Zombie(texZombie, 800, 300));
    // Agregamos otro zombie mas rapido en otra linea para probar
    Zombie zombieRapido(texZombie, 800, 100);
    zombieRapido.velocidad = 3.0f; // Modificamos su propiedad publica
    hordaZombies.push_back(zombieRapido);


    // --- GAME LOOP ---
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // EVENTO: Clic para poner planta
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    // Creamos una planta y la metemos al vector "jardin"
                    // Pasamos la textura por referencia para no cargar la imagen mil veces
                    jardin.push_back(Planta(texPlanta, event.mouseButton.x, event.mouseButton.y));
                }
            }
        }

        // --- UPDATE (ACTUALIZAR TODO) ---
        // Recorremos el vector de zombies y le decimos a cada uno "muevete"
        for (int i = 0; i < hordaZombies.size(); i++) {
            hordaZombies[i].update();
        }

        // --- RENDER (DIBUJAR TODO) ---
        window.clear(sf::Color::Green); // Limpiar

        // 1. Dibujar Plantas
        for (int i = 0; i < jardin.size(); i++) {
            jardin[i].draw(window);
        }

        // 2. Dibujar Zombies
        for (int i = 0; i < hordaZombies.size(); i++) {
            hordaZombies[i].draw(window);
        }

        window.display();
    }
    return 0;
}