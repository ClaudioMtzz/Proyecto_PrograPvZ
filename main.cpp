#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <iostream>
#include <cmath> 

// --- CONSTANTES ---
const int ANCHO_CELDA = 80;
const int ALTO_CELDA = 100;
const int OFFSET_X = 30; 
const int OFFSET_Y = 80; 

// --- CLASE PROYECTIL ---
class Proyectil {
public:
    sf::Sprite shape;
    float velocidad;
    bool borrar;
    bool esHielo;

    Proyectil(sf::Texture &textura, float x, float y, bool hielo) {
        shape.setTexture(textura);
        shape.setPosition(x, y);
        shape.setScale(0.1f, 0.1f); 
        velocidad = 7.0f;
        borrar = false;
        esHielo = hielo;

        if (esHielo) {
            shape.setColor(sf::Color::Cyan); 
        }
    }

    void update() {
        shape.move(velocidad, 0);
        if (shape.getPosition().x > 850) borrar = true;
    }
};

// --- CLASE PLANTA ---
class Planta {
public:
    sf::Sprite shape;
    sf::Clock relojDisparo;    
    sf::Clock relojProduccion; 
    int tipo; 
    int vida;

    Planta(sf::Texture &textura, float x, float y, int tipoPlanta) {
        tipo = tipoPlanta;
        shape.setTexture(textura);
        shape.setPosition(x + 5, y + 10); 

        if (tipo == 1) { // LANZAGUISANTES
            shape.setScale(0.18f, 0.18f); 
            vida = 5;  
        } else if (tipo == 2) { // NUEZ
            shape.setScale(0.08f, 0.08f); 
            vida = 30; 
        } else if (tipo == 3) { // GIRASOL
            shape.setScale(0.15f, 0.15f); 
            vida = 5; 
        } else if (tipo == 4) { // HIELAGUISANTES
            shape.setScale(0.18f, 0.18f); 
            vida = 6;
            shape.setColor(sf::Color(100, 255, 255)); 
        }
    }

    bool estaCargada() {
        if (tipo != 1 && tipo != 4) return false; 
        return relojDisparo.getElapsedTime().asSeconds() > 1.5f;
    }

    void recargar() {
        relojDisparo.restart();
    }

    int producirSoles() {
        if (tipo == 3) { 
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
    float velocidadBase;   
    float velocidadActual; 
    int vida;
    bool estaComiendo;
    bool congelado;        
    sf::Clock relojMordida; 
    sf::Clock relojCongelado;

    Zombie(sf::Texture &textura, float x, float y, int nivel) {
        shape.setTexture(textura);
        shape.setPosition(x, y);
        shape.setScale(0.16f, 0.16f); 
        estaComiendo = false;
        congelado = false;

        if (nivel == 1) { // NORMAL
            vida = 5;
            velocidadBase = 0.5f;
        } else if (nivel == 2) { // CUBETA
            vida = 15; 
            velocidadBase = 0.4f; 
            shape.setColor(sf::Color(200, 200, 255)); 
        } else if (nivel == 3) { // FUTBOLISTA
            // !!! CAMBIO 1: NERFEO AL FUTBOLISTA !!!
            vida = 25; // Bajamos de 40 a 25. Mas justo.
            velocidadBase = 1.3f; 
            shape.setColor(sf::Color(255, 100, 100)); 
        }
        velocidadActual = velocidadBase;
    }

    void aplicarHielo() {
        if (!congelado) {
            congelado = true;
            velocidadActual = velocidadBase * 0.5f; 
            shape.setColor(sf::Color::Cyan); 
        }
        relojCongelado.restart(); 
    }

    bool update(std::vector<Planta> &jardin, sf::Sound &sonidoComer) {
        if (congelado && relojCongelado.getElapsedTime().asSeconds() > 2.0f) {
            congelado = false;
            velocidadActual = velocidadBase; 
            shape.setColor(sf::Color::White); 
        }

        estaComiendo = false; 
        for (auto &planta : jardin) {
            if (planta.vida > 0 && shape.getGlobalBounds().intersects(planta.shape.getGlobalBounds())) {
                estaComiendo = true; 
                if (relojMordida.getElapsedTime().asSeconds() > 0.5f) {
                    planta.vida--; 
                    float tonoRandom = 0.8f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.4f));
                    sonidoComer.setPitch(tonoRandom); 
                    sonidoComer.play();
                    relojMordida.restart();
                }
                break; 
            }
        }
        if (!estaComiendo) {
            shape.move(-velocidadActual, 0); 
        }
        if (shape.getPosition().x < 10) return true; 
        return false;
    }
};

// --- MAIN ---
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "PvZ - Fase 15: Pala y Precios");
    window.setFramerateLimit(60);

    // --- CARGAR RECURSOS ---
    sf::Texture texFondo, texZombie, texPlanta, texNuez, texGirasol, texBala, texHiela, texFutbol, texBalaHielo, texPala;
    texFondo.loadFromFile("fondo.png"); 
    
    if (!texZombie.loadFromFile("zombie.png")) return -1;
    if (!texPlanta.loadFromFile("planta.png")) return -1;
    if (!texNuez.loadFromFile("nuez.png")) return -1; 
    if (!texGirasol.loadFromFile("girasol.png")) return -1;
    if (!texBala.loadFromFile("bala.png")) return -1;
    
    if (!texHiela.loadFromFile("hiela.png")) texHiela = texPlanta; 
    if (!texFutbol.loadFromFile("futbol.png")) texFutbol = texZombie; 
    if (!texBalaHielo.loadFromFile("hielo.png")) texBalaHielo = texBala;
    // Cargar Pala (con fallback)
    if (!texPala.loadFromFile("pala.png")) texPala = texZombie; 

    sf::Font fuente;
    if (!fuente.loadFromFile("arial.ttf")) return -1;

    // AUDIO
    sf::Music musicaFondo;
    if (musicaFondo.openFromFile("musica.ogg")) {
        musicaFondo.setLoop(true); musicaFondo.setVolume(40); musicaFondo.play();        
    }
    sf::SoundBuffer bufferPiu, bufferGolpe, bufferComer, bufferGameOver;
    sf::Sound sonidoPiu, sonidoGolpe, sonidoComer, sonidoGameOver;
    
    if (bufferPiu.loadFromFile("piu.wav")) sonidoPiu.setBuffer(bufferPiu);
    if (bufferGolpe.loadFromFile("golpe.wav")) sonidoGolpe.setBuffer(bufferGolpe);
    if (bufferComer.loadFromFile("comer.wav")) { sonidoComer.setBuffer(bufferComer); sonidoComer.setVolume(80); }
    if (bufferGameOver.loadFromFile("gameover.wav")) { sonidoGameOver.setBuffer(bufferGameOver); sonidoGameOver.setVolume(100); }

    // VISUAL
    sf::Sprite spriteFondo(texFondo);
    if(texFondo.getSize().x > 0) spriteFondo.setScale(800.0f / texFondo.getSize().x, 600.0f / texFondo.getSize().y);

    sf::Text textoSoles;
    textoSoles.setFont(fuente); textoSoles.setCharacterSize(24); textoSoles.setFillColor(sf::Color::Black); textoSoles.setPosition(10, 10);

    sf::Text textoGameOver;
    textoGameOver.setFont(fuente); textoGameOver.setString("GAME OVER\nPresiona R");
    textoGameOver.setCharacterSize(50); textoGameOver.setFillColor(sf::Color::Red); textoGameOver.setStyle(sf::Text::Bold);
    sf::FloatRect tr = textoGameOver.getLocalBounds(); textoGameOver.setOrigin(tr.width/2, tr.height/2); textoGameOver.setPosition(400, 300);

    // --- TIENDA Y PRECIOS ---
    int plantaSeleccionada = 1; // 0 = Pala, 1-4 = Plantas

    // Botones
    sf::RectangleShape btnGuisante(sf::Vector2f(60, 60)); btnGuisante.setPosition(150, 10); btnGuisante.setFillColor(sf::Color::Green); 
    sf::Sprite iconoGuisante(texPlanta); iconoGuisante.setPosition(155, 15); iconoGuisante.setScale(0.1f, 0.1f);
    
    sf::RectangleShape btnNuez(sf::Vector2f(60, 60)); btnNuez.setPosition(220, 10); btnNuez.setFillColor(sf::Color(150, 100, 50)); 
    sf::Sprite iconoNuez(texNuez); iconoNuez.setPosition(225, 15); iconoNuez.setScale(0.06f, 0.06f); 
    
    sf::RectangleShape btnGirasol(sf::Vector2f(60, 60)); btnGirasol.setPosition(290, 10); btnGirasol.setFillColor(sf::Color::Yellow); 
    sf::Sprite iconoGirasol(texGirasol); iconoGirasol.setPosition(295, 15); iconoGirasol.setScale(0.1f, 0.1f);
    
    sf::RectangleShape btnHiela(sf::Vector2f(60, 60)); btnHiela.setPosition(360, 10); btnHiela.setFillColor(sf::Color::Cyan); 
    sf::Sprite iconoHiela(texHiela); iconoHiela.setPosition(365, 15); iconoHiela.setScale(0.1f, 0.1f); iconoHiela.setColor(sf::Color(150, 255, 255));

    // !!! NUEVO BOTON: PALA !!!
    sf::RectangleShape btnPala(sf::Vector2f(60, 60)); btnPala.setPosition(430, 10); btnPala.setFillColor(sf::Color(100, 100, 100)); // Gris
    sf::Sprite iconoPala(texPala); iconoPala.setPosition(435, 15); iconoPala.setScale(0.1f, 0.1f); // Ajusta escala si es necesario

    // !!! CAMBIO 2: ETIQUETAS DE PRECIO !!!
    sf::Text txtP1, txtP2, txtP3, txtP4;
    txtP1.setFont(fuente); txtP1.setString("$50");  txtP1.setCharacterSize(16); txtP1.setPosition(160, 70);
    txtP2.setFont(fuente); txtP2.setString("$50");  txtP2.setCharacterSize(16); txtP2.setPosition(230, 70);
    txtP3.setFont(fuente); txtP3.setString("$50");  txtP3.setCharacterSize(16); txtP3.setPosition(300, 70);
    txtP4.setFont(fuente); txtP4.setString("$175"); txtP4.setCharacterSize(16); txtP4.setPosition(370, 70);

    // VARIABLES
    std::vector<Planta> jardin;
    std::vector<Zombie> horda;
    std::vector<Proyectil> municion;
    int dineroSoles = 150; 
    bool juegoTerminado = false;
    sf::Clock relojIngresoPasivo;
    sf::Clock relojGeneradorZombies;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (juegoTerminado && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                juegoTerminado = false; jardin.clear(); horda.clear(); municion.clear();
                dineroSoles = 150; relojGeneradorZombies.restart();
                if (musicaFondo.getStatus() == sf::Music::Stopped) musicaFondo.play();
            }

            if (!juegoTerminado && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i pp = sf::Mouse::getPosition(window);
                    sf::Vector2f wp = window.mapPixelToCoords(pp);
                    int mx = static_cast<int>(wp.x); int my = static_cast<int>(wp.y);

                    // Seleccion en tienda
                    if (btnGuisante.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 1;
                    else if (btnNuez.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 2;
                    else if (btnGirasol.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 3;
                    else if (btnHiela.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 4; 
                    else if (btnPala.getGlobalBounds().contains(mx, my)) plantaSeleccionada = 0; // 0 = MODO PALA

                    // Clic en el jardin
                    else if (my > OFFSET_Y) { 
                        int col = (mx - OFFSET_X) / ANCHO_CELDA;
                        int row = (my - OFFSET_Y) / ALTO_CELDA;
                        float posX = col * ANCHO_CELDA + OFFSET_X;
                        float posY = row * ALTO_CELDA + OFFSET_Y;
                        
                        // !!! CAMBIO 3: LOGICA DE LA PALA (Borrar) !!!
                        if (plantaSeleccionada == 0) {
                            // Buscar si hay planta en esta celda y borrarla
                            for(auto it = jardin.begin(); it != jardin.end(); ++it) {
                                int pCol = (it->shape.getPosition().x - OFFSET_X) / ANCHO_CELDA;
                                int pRow = (it->shape.getPosition().y - OFFSET_Y) / ALTO_CELDA;
                                if(pCol == col && pRow == row) {
                                    jardin.erase(it); // Adios planta
                                    // Opcional: Podrias recuperar la mitad del costo aqui
                                    break; // Ya borramos, salimos del loop
                                }
                            }
                        }
                        // LOGICA DE PLANTAR (Solo si no es pala)
                        else {
                            int costo = 50; 
                            if (plantaSeleccionada == 4) costo = 175; 

                            if (dineroSoles >= costo && col < 9 && col >= 0) {
                                bool celdaOcupada = false;
                                for(auto &p : jardin) {
                                    int pCol = (p.shape.getPosition().x - OFFSET_X) / ANCHO_CELDA;
                                    int pRow = (p.shape.getPosition().y - OFFSET_Y) / ALTO_CELDA;
                                    if(pCol == col && pRow == row) { celdaOcupada = true; break; }
                                }

                                if (!celdaOcupada) {
                                    sf::Texture* tU = &texPlanta; 
                                    if (plantaSeleccionada == 2) tU = &texNuez;
                                    else if (plantaSeleccionada == 3) tU = &texGirasol;
                                    else if (plantaSeleccionada == 4) tU = &texHiela;
                                    jardin.push_back(Planta(*tU, posX, posY, plantaSeleccionada));
                                    dineroSoles -= costo;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!juegoTerminado) {
            // Feedback visual de seleccion (incluyendo la pala)
            btnGuisante.setOutlineThickness(plantaSeleccionada == 1 ? 3 : 0);
            btnNuez.setOutlineThickness(plantaSeleccionada == 2 ? 3 : 0);
            btnGirasol.setOutlineThickness(plantaSeleccionada == 3 ? 3 : 0);
            btnHiela.setOutlineThickness(plantaSeleccionada == 4 ? 3 : 0);
            btnPala.setOutlineThickness(plantaSeleccionada == 0 ? 3 : 0); // Resaltar pala

            if (relojIngresoPasivo.getElapsedTime().asSeconds() > 10.0f) { 
                dineroSoles += 10; relojIngresoPasivo.restart();
            }
            for (auto &planta : jardin) {
                if(planta.producirSoles() > 0) dineroSoles += 25;
            }
            textoSoles.setString("Soles: " + std::to_string(dineroSoles));

            if (relojGeneradorZombies.getElapsedTime().asSeconds() > 5.0f) {
                int filaRandom = rand() % 5; 
                float posZ_Y = filaRandom * ALTO_CELDA + OFFSET_Y + 10;
                
                int r = rand() % 10; 
                int tipoZombie = 1; 
                if (r >= 5 && r < 7) tipoZombie = 2; 
                else if (r >= 7) tipoZombie = 3;     

                horda.push_back(Zombie((tipoZombie == 3 ? texFutbol : texZombie), 900, posZ_Y, tipoZombie));
                relojGeneradorZombies.restart();
            }

            for (auto &zombie : horda) {
                if (zombie.update(jardin, sonidoComer)) {
                    if (!juegoTerminado) { juegoTerminado = true; musicaFondo.stop(); sonidoGameOver.play(); }
                }
            }

            for (auto it = jardin.begin(); it != jardin.end(); ) {
                if (it->vida <= 0) it = jardin.erase(it); else ++it;
            }
            for (auto &bala : municion) bala.update();

            for (auto &planta : jardin) {
                if (planta.estaCargada()) {
                    bool enemigo = false;
                    for (auto &zombie : horda) {
                        if (std::abs(planta.shape.getPosition().y - zombie.shape.getPosition().y) < 15 &&
                            zombie.shape.getPosition().x > planta.shape.getPosition().x &&
                            zombie.shape.getPosition().x < 850) { enemigo = true; break; }
                    }
                    if (enemigo) {
                        bool esHielo = (planta.tipo == 4);
                        municion.push_back(Proyectil((esHielo ? texBalaHielo : texBala), 
                                           planta.shape.getPosition().x + 40, planta.shape.getPosition().y + 10, esHielo));
                        planta.recargar();
                        sonidoPiu.play(); 
                    }
                }
            }

            for (auto itBala = municion.begin(); itBala != municion.end(); ) {
                bool choco = false;
                for (auto itZombie = horda.begin(); itZombie != horda.end(); ) {
                    if (itBala->shape.getGlobalBounds().intersects(itZombie->shape.getGlobalBounds())) {
                        itZombie->vida--;
                        if (itBala->esHielo) itZombie->aplicarHielo();

                        choco = true;
                        sonidoGolpe.play();
                        if (itZombie->vida <= 0) {
                            int premio = 25;
                            if (itZombie->velocidadBase > 1.0f) premio = 75; 
                            else if (itZombie->velocidadBase < 0.5f) premio = 50; 
                            dineroSoles += premio; 
                            itZombie = horda.erase(itZombie);
                        } else { ++itZombie; }
                        break; 
                    } else { ++itZombie; }
                }
                if (choco || itBala->borrar) itBala = municion.erase(itBala);
                else ++itBala;
            }
        }

        window.clear();
        window.draw(spriteFondo); 
        // Dibujar tienda y pala
        window.draw(btnGuisante); window.draw(iconoGuisante);
        window.draw(btnNuez);     window.draw(iconoNuez);
        window.draw(btnGirasol);  window.draw(iconoGirasol);
        window.draw(btnHiela);    window.draw(iconoHiela); 
        window.draw(btnPala);     window.draw(iconoPala);
        window.draw(textoSoles);
        // Dibujar precios
        window.draw(txtP1); window.draw(txtP2); window.draw(txtP3); window.draw(txtP4);

        for (auto &planta : jardin) window.draw(planta.shape);
        for (auto &zombie : horda) window.draw(zombie.shape);
        for (auto &bala : municion) window.draw(bala.shape);

        if (juegoTerminado) {
             sf::RectangleShape cortina(sf::Vector2f(3000, 3000)); cortina.setFillColor(sf::Color(0,0,0, 200)); 
             window.draw(cortina); window.draw(textoGameOver); 
        }
        window.display();
    }
    return 0;
}