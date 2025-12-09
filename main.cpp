#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath> // IMPORTANTE: Para calcular distancias (std::abs)

// --- CONSTANTES DE CONFIGURACION ---
const int ANCHO_CELDA = 80;
const int ALTO_CELDA = 100;
const int OFFSET_X = 30; // Margen izquierdo
const int OFFSET_Y = 80; // Margen superior (Espacio para la tienda)

// --- CLASE PROYECTIL ---
class Proyectil {
public:
    sf::Sprite shape;
    float velocidad;
    bool borrar;

    Proyectil(sf::Texture &textura, float x, float y) {
        shape.setTexture(textura);
        shape.setPosition(x, y);
        shape.setScale(0.1f, 0.1f); // Ajusta si tu bala es muy grande
        velocidad = 7.0f;
        borrar = false;
    }

    void update() {
        shape.move(velocidad, 0);
        // Si sale de la pantalla (850px), marcar para borrar
        if (shape.getPosition().x > 850) borrar = true;
    }
};

// --- CLASE PLANTA ---
class Planta {
public:
    sf::Sprite shape;
    sf::Clock relojDisparo;     // Cronometro para atacar
    sf::Clock relojProduccion;  // Cronometro para generar dinero (Girasol)
    int tipo; // 1=Guisante, 2=Nuez, 3=Girasol
    int vida;

    Planta(sf::Texture &textura, float x, float y, int tipoPlanta) {
        tipo = tipoPlanta;
        shape.setTexture(textura);
        shape.setPosition(x + 5, y + 10); // Centrado ligero en la celda

        // CONFIGURACION SEGUN EL TIPO
        if (tipo == 1) { // LANZAGUISANTES
            shape.setScale(0.15f, 0.15f); 
            vida = 5;  
        } else if (tipo == 2) { // NUEZ (Tanque)
            shape.setScale(0.12f, 0.12f); // Tamaño corregido (pequeño)
            vida = 30; // Aguanta 30 mordidas
        } else if (tipo == 3) { // GIRASOL (Economia)
            shape.setScale(0.15f, 0.15f); 
            vida = 5; // Debil
        }
    }

    // -- SISTEMA DE DISPARO INTELIGENTE --
    
    // Solo nos dice si el arma esta lista por tiempo (Cooldown)
    bool estaCargada() {
        if (tipo != 1) return false; // Solo el Guisante dispara
        return relojDisparo.getElapsedTime().asSeconds() > 1.5f;
    }

    // Reinicia el reloj (Se llama manualmente cuando decidimos disparar)
    void recargar() {
        relojDisparo.restart();
    }

    // -- SISTEMA DE ECONOMIA --
    int producirSoles() {
        if (tipo == 3) { // Solo Girasol
            // Produce 25 soles cada 10 segundos (puedes bajarlo a 5.0f para probar)
            if (relojProduccion.getElapsedTime().asSeconds() > 5.0f) {
                relojProduccion.restart();
                return 25; 
            }
        }
        return 0; 
    }
};

// --- CLASE ZOMBIE ---
class Zombie {
public:
    sf::Sprite shape;
    float velocidadNormal;
    int vida;
    bool estaComiendo;
    sf::Clock relojMordida; 

    Zombie(sf::Texture &textura, float x, float y, int nivel) {
        shape.setTexture(textura);
        shape.setPosition(x, y);
        shape.setScale(0.15f, 0.15f);
        estaComiendo = false;

        if (nivel == 1) { // NORMAL
            vida = 5;
            velocidadNormal = 0.5f;
        } else if (nivel == 2) { // CUBETA (Buckethead)
            vida = 15; // Triple vida
            velocidadNormal = 0.4f; // Un poco mas lento
            shape.setColor(sf::Color(200, 200, 255)); // Tinte azul
        }
    }

    // Retorna TRUE si llega a la casa (Game Over)
    bool update(std::vector<Planta> &jardin) {
        estaComiendo = false; 

        // 1. Detectar colision con plantas
        for (auto &planta : jardin) {
            if (planta.vida > 0 && shape.getGlobalBounds().intersects(planta.shape.getGlobalBounds())) {
                estaComiendo = true; 
                // Comer cada 0.5 segundos
                if (relojMordida.getElapsedTime().asSeconds() > 0.5f) {
                    planta.vida--; 
                    relojMordida.restart();
                }
                break; // Solo come una planta a la vez
            }
        }

        // 2. Moverse si no esta comiendo
        if (!estaComiendo) {
            shape.move(-velocidadNormal, 0); 
        }

        // 3. Game Over check
        if (shape.getPosition().x < 10) return true; 
        return false;
    }
};

// --- MAIN (EL CEREBRO DEL JUEGO) ---
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "PvZ - Proyecto Final Mecatronica");
    window.setFramerateLimit(60);

    // 1. CARGAR RECURSOS (Assets)
    sf::Texture texFondo, texZombie, texPlanta, texNuez, texGirasol, texBala;
    
    // Carga segura (evita crash inmediato si falta el fondo)
    texFondo.loadFromFile("fondo.png"); 
    
    // Validacion critica
    if (!texZombie.loadFromFile("zombie.png")) return -1;
    if (!texPlanta.loadFromFile("planta.png")) return -1;
    if (!texNuez.loadFromFile("nuez.png")) return -1; 
    if (!texGirasol.loadFromFile("girasol.png")) return -1;
    if (!texBala.loadFromFile("bala.png")) return -1;
    
    sf::Font fuente;
    if (!fuente.loadFromFile("arial.ttf")) {
        std::cout << "ERROR: Falta arial.ttf" << std::endl;
        return -1;
    }

    // Configurar sprite de fondo
    sf::Sprite spriteFondo(texFondo);
    if(texFondo.getSize().x > 0)
        spriteFondo.setScale(800.0f / texFondo.getSize().x, 600.0f / texFondo.getSize().y);

    // Textos UI
    sf::Text textoSoles;
    textoSoles.setFont(fuente);
    textoSoles.setCharacterSize(24);
    textoSoles.setFillColor(sf::Color::Black); 
    textoSoles.setPosition(10, 10);

    sf::Text textoGameOver;
    textoGameOver.setFont(fuente);
    textoGameOver.setString("GAME OVER\nPresiona R para reiniciar");
    textoGameOver.setCharacterSize(50);
    textoGameOver.setFillColor(sf::Color::Red);
    textoGameOver.setStyle(sf::Text::Bold);
    // Centrar texto
    sf::FloatRect textRect = textoGameOver.getLocalBounds();
    textoGameOver.setOrigin(textRect.width/2, textRect.height/2);
    textoGameOver.setPosition(400, 300);

    // --- TIENDA (BOTONES) ---
    int plantaSeleccionada = 1; 
    
    // Boton 1: Guisante
    sf::RectangleShape btnGuisante(sf::Vector2f(60, 60));
    btnGuisante.setPosition(150, 10);
    btnGuisante.setFillColor(sf::Color::Green); 
    sf::Sprite iconoGuisante(texPlanta);
    iconoGuisante.setPosition(155, 15);
    iconoGuisante.setScale(0.1f, 0.1f);

    // Boton 2: Nuez
    sf::RectangleShape btnNuez(sf::Vector2f(60, 60));
    btnNuez.setPosition(220, 10);
    btnNuez.setFillColor(sf::Color(150, 100, 50)); // Cafe
    sf::Sprite iconoNuez(texNuez);
    iconoNuez.setPosition(225, 15);
    iconoNuez.setScale(0.1f, 0.1f); // Icono pequeño

    // Boton 3: Girasol
    sf::RectangleShape btnGirasol(sf::Vector2f(60, 60));
    btnGirasol.setPosition(290, 10);
    btnGirasol.setFillColor(sf::Color::Yellow); 
    sf::Sprite iconoGirasol(texGirasol);
    iconoGirasol.setPosition(295, 15);
    iconoGirasol.setScale(0.1f, 0.1f);

    // --- VARIABLES DE JUEGO ---
    std::vector<Planta> jardin;
    std::vector<Zombie> horda;
    std::vector<Proyectil> municion;
    
    int dineroSoles = 100; // Inicias con lo justo para 1 planta
    bool juegoTerminado = false;

    sf::Clock relojIngresoPasivo;
    sf::Clock relojGeneradorZombies;

    // --- GAME LOOP ---
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // REINICIO
            if (juegoTerminado && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                juegoTerminado = false;
                jardin.clear(); horda.clear(); municion.clear();
                dineroSoles = 100;
                relojGeneradorZombies.restart();
            }

            // CLICS DEL MOUSE
            if (!juegoTerminado && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int mx = event.mouseButton.x;
                    int my = event.mouseButton.y;

                    // A. SELECCION EN TIENDA
                    if (btnGuisante.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 1;
                    else if (btnNuez.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 2;
                    else if (btnGirasol.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 3;
                    
                    // B. COLOCAR EN JARDIN
                    else if (my > OFFSET_Y) { 
                        // Calculo de celda (Grid)
                        int col = (mx - OFFSET_X) / ANCHO_CELDA;
                        float posX = col * ANCHO_CELDA + OFFSET_X;
                        // Ajuste Y para alinear filas
                        float posY = ((my - OFFSET_Y) / ALTO_CELDA) * ALTO_CELDA + OFFSET_Y;
                        
                        int costo = 50; // Precio fijo para este prototipo

                        if (dineroSoles >= costo && col < 9 && col >= 0) {
                            sf::Texture& texturaUsar = (plantaSeleccionada == 1) ? texPlanta : (plantaSeleccionada == 2 ? texNuez : texGirasol);
                            jardin.push_back(Planta(texturaUsar, posX, posY, plantaSeleccionada));
                            dineroSoles -= costo;
                        }
                    }
                }
            }
        }

        // --- UPDATE (LOGICA) ---
        if (!juegoTerminado) {
            // UI Feedback (Borde blanco en seleccion)
            btnGuisante.setOutlineThickness(plantaSeleccionada == 1 ? 3 : 0);
            btnNuez.setOutlineThickness(plantaSeleccionada == 2 ? 3 : 0);
            btnGirasol.setOutlineThickness(plantaSeleccionada == 3 ? 3 : 0);

            // ECONOMIA
            // Ingreso pasivo de emergencia (muy lento)
            if (relojIngresoPasivo.getElapsedTime().asSeconds() > 15.0f) { 
                dineroSoles += 10;
                relojIngresoPasivo.restart();
            }
            // Produccion de Girasoles
            for (auto &planta : jardin) {
                int ganancia = planta.producirSoles();
                if (ganancia > 0) dineroSoles += ganancia;
            }
            textoSoles.setString("Soles: " + std::to_string(dineroSoles));

            // GENERADOR DE ZOMBIES
            if (relojGeneradorZombies.getElapsedTime().asSeconds() > 5.0f) {
                int filaRandom = rand() % 5; 
                float posZ_Y = filaRandom * ALTO_CELDA + OFFSET_Y + 10;
                // 20% Probabilidad de Zombie Cubeta
                int tipoZombie = (rand() % 5 == 0) ? 2 : 1; 
                horda.push_back(Zombie(texZombie, 900, posZ_Y, tipoZombie));
                relojGeneradorZombies.restart();
            }

            // ACTUALIZAR ZOMBIES
            for (auto &zombie : horda) {
                if (zombie.update(jardin)) juegoTerminado = true; // Activa Game Over
            }

            // LIMPIEZA DE PLANTAS MUERTAS
            for (auto it = jardin.begin(); it != jardin.end(); ) {
                if (it->vida <= 0) it = jardin.erase(it);
                else ++it;
            }

            // MOVER BALAS
            for (auto &bala : municion) bala.update();

            // DISPARO INTELIGENTE (Detectar carril)
            for (auto &planta : jardin) {
                if (planta.estaCargada()) {
                    bool enemigoALaVista = false;
                    for (auto &zombie : horda) {
                        // 1. Misma linea Y (con tolerancia de 10px)
                        bool mismaFila = std::abs(planta.shape.getPosition().y - zombie.shape.getPosition().y) < 15;
                        // 2. Zombie adelante de la planta
                        bool adelante = zombie.shape.getPosition().x > planta.shape.getPosition().x;
                        // 3. Zombie dentro de pantalla
                        bool enPantalla = zombie.shape.getPosition().x < 850;

                        if (mismaFila && adelante && enPantalla) {
                            enemigoALaVista = true;
                            break;
                        }
                    }
                    if (enemigoALaVista) {
                        municion.push_back(Proyectil(texBala, planta.shape.getPosition().x + 40, planta.shape.getPosition().y + 10));
                        planta.recargar();
                    }
                }
            }

            // COLISIONES (Balas vs Zombies)
            for (auto itBala = municion.begin(); itBala != municion.end(); ) {
                bool choco = false;
                for (auto itZombie = horda.begin(); itZombie != horda.end(); ) {
                    if (itBala->shape.getGlobalBounds().intersects(itZombie->shape.getGlobalBounds())) {
                        itZombie->vida--;
                        choco = true;
                        // Si muere el zombie
                        if (itZombie->vida <= 0) {
                            dineroSoles += (itZombie->velocidadNormal < 0.5) ? 50 : 25; // Cubeta da mas dinero
                            itZombie = horda.erase(itZombie);
                        } else { ++itZombie; }
                        break; 
                    } else { ++itZombie; }
                }
                if (choco || itBala->borrar) itBala = municion.erase(itBala);
                else ++itBala;
            }
        }

        // --- RENDER (DIBUJAR) ---
        window.clear();
        
        window.draw(spriteFondo); 
        
        // UI Tienda
        window.draw(btnGuisante); window.draw(iconoGuisante);
        window.draw(btnNuez);     window.draw(iconoNuez);
        window.draw(btnGirasol);  window.draw(iconoGirasol);
        window.draw(textoSoles);

        // Objetos de Juego
        for (auto &planta : jardin) window.draw(planta.shape);
        for (auto &zombie : horda) window.draw(zombie.shape);
        for (auto &bala : municion) window.draw(bala.shape);

        // Pantalla de Game Over
        if (juegoTerminado) {
             sf::RectangleShape cortina(sf::Vector2f(800, 600));
             cortina.setFillColor(sf::Color(0,0,0, 200)); 
             window.draw(cortina);
             window.draw(textoGameOver); 
        }

        window.display();
    }
    return 0;
}