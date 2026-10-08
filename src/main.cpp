// ./src/main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include "Cercle.hpp"
#include "Coup.hpp"
#include "TileMap.hpp"
#include "Escouade.hpp"
#include "Monstre.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Squad-ARPG");
    window.setFramerateLimit(60);
    // Une touche maintenue ne produit qu'un seul événement d'appui.
    window.setKeyRepeatEnabled(false);
    const std::vector<std::string> plan = {
        "###########################################################",
        "#.........#..................###########..................#",
        "#.........#..................###########..................#",
        "#.........#...............................................#",
        "#.........#...............................................#",
        "#.........#.......####....................................#",
        "#.........#.......####.......###########..................#",
        "#.........#..................###########..................#",
        "#............................####################...#######",
        "#............................#............................#",
        "#............................#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "#.........#.......####.......#............................#",
        "#.........#.......####.......#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "##############################################...##########",
        "##############################################...##########",
        "#............................######.......................#",
        "#............................######.......................#",
        "#.....####...................#............................#",
        "#.....####...................#............................#",
        "#.................####.......#............................#",
        "#.................####.......######.......................#",
        "#............................######.......................#",
        "#............................####################...#######",
        "#............................#............................#",
        "#............................#............................#",
        "###########..................#............................#",
        "#.........#..................#............................#",
        "#.........#.......####.................###................#",
        "#.........#.......####.................###................#",
        "#......................................###.........########",
        "#............................#............................#",
        "#............................#............................#",
        "#.........#..................#............................#",
        "###########################################################",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({100.0f, 100.0f});
    int prochainId = 1;
    std::vector<Monstre> monstres = {
        Monstre({660.0f, 380.0f}, prochainId++),
        Monstre({500.0f, 580.0f}, prochainId++),
        Monstre({900.0f, 220.0f}, prochainId++),
        Monstre({1940.0f, 180.0f}, prochainId++),
        Monstre({1500.0f, 540.0f}, prochainId++),
        Monstre({1620.0f, 540.0f}, prochainId++),
    };
    bool deplacementContinu = false;

    sf::Clock horloge;
    sf::View camera = window.getDefaultView();

    while (window.isOpen()) {
        const float dt = horloge.restart().asSeconds();
        std::optional<std::size_t> placeSkillDemandee;
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (data->button == sf::Mouse::Button::Left && escouade.estVivante()) {
                    const sf::Vector2f clicMonde = window.mapPixelToCoords(data->position);
                    const auto it = std::find_if(monstres.begin(), monstres.end(), [&](const Monstre& m) {
                        return m.contient(clicMonde);
                    });
                    if (it != monstres.end()) {
                        escouade.verrouiller(it->getId());
                        deplacementContinu = false;
                    } else {
                        escouade.deverrouiller();
                        deplacementContinu = true;
                    }
                }
            }
            if (const auto* data = event->getIf<sf::Event::KeyPressed>()) {
                // Scancode : la touche physique « 1 » de la rangée du haut,
                // quelle que soit la disposition du clavier (AZERTY : « & »).
                // Touche N = personnage à la place N de la formation.
                if (data->scancode == sf::Keyboard::Scan::Num1) {
                    placeSkillDemandee = 1;
                } else if (data->scancode == sf::Keyboard::Scan::Num2) {
                    placeSkillDemandee = 2;
                } else if (data->scancode == sf::Keyboard::Scan::Num3) {
                    placeSkillDemandee = 3;
                } else if (data->scancode == sf::Keyboard::Scan::Num4) {
                    placeSkillDemandee = 4;
                }
            }
        }

        // Monstre verrouillé : monstres.end() s'il n'y en a pas.
        auto monstreVerrouille = monstres.end();
        if (const std::optional<int> idCible = escouade.getCibleVerrouillee(); idCible.has_value()) {
            monstreVerrouille = std::find_if(monstres.begin(), monstres.end(), [&](const Monstre& m) {
                return m.getId() == idCible.value();
            });
            if (monstreVerrouille == monstres.end()) {
                escouade.deverrouiller();
            }
        }

        // Le skill passe avant l'attaque de base : s'il part, il relance
        // la recharge de base de ce personnage, qui ne frappe donc pas une
        // seconde fois dans la même image. Un soin ne rend aucun dégât.
        if (placeSkillDemandee.has_value()) {
            std::optional<Cercle> cercleCible;
            if (monstreVerrouille != monstres.end()) {
                cercleCible = monstreVerrouille->getCercle();
            }
            const int degats = escouade.declencherSkill(*placeSkillDemandee, cercleCible);
            if (monstreVerrouille != monstres.end()) {
                monstreVerrouille->recevoirDegats(degats);
            }
        }

        // Les coups sont demandés à chaque image, verrou ou non : chaque
        // personnage vivant frappe de lui-même un monstre à sa portée.
        if (escouade.estVivante()) {
            std::vector<MonstreVu> monstresVus;
            for (const Monstre& m : monstres) {
                monstresVus.push_back({m.getId(), m.getCercle()});
            }
            for (const Coup& coup : escouade.attaquer(monstresVus, carte)) {
                const auto touche = std::find_if(monstres.begin(), monstres.end(), [&](const Monstre& m) {
                    return m.getId() == coup.idMonstre;
                });
                touche->recevoirDegats(coup.degats);
            }
            if (monstreVerrouille != monstres.end() && !monstreVerrouille->estVivant()) {
                escouade.deverrouiller();
            }
        }

        std::erase_if(monstres, [](const Monstre& m) { return !m.estVivant(); });

        if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && deplacementContinu && escouade.estVivante()) {
            const sf::Vector2f curseurMonde = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            escouade.allerVers(curseurMonde, carte);
        }

        // MISE À JOUR
        // Les monstres (à leur position de l'image précédente) sont des obstacles pour les personnages.
        std::vector<Cercle> cerclesMonstres;
        for (const Monstre& m : monstres) {
            cerclesMonstres.push_back(m.getCercle());
        }
        escouade.mettreAJour(dt, carte, cerclesMonstres);

        // Les personnages vivants (à leur position à jour) sont des obstacles pour tous les monstres.
        const std::vector<Cercle> cerclesPersonnages = escouade.cerclesDesVivants();

        for (Monstre& m : monstres) {
            // Les autres monstres, à leur position courante, sont aussi des obstacles.
            std::vector<Cercle> obstacles = cerclesPersonnages;
            for (const Monstre& autre : monstres) {
                if (autre.getId() != m.getId()) {
                    obstacles.push_back(autre.getCercle());
                }
            }
            const auto cible = escouade.personnageLePlusProche(m.getCentre());
            m.mettreAJour(dt, cible, carte, obstacles);
            const sf::Vector2f origine = m.getCentre();
            if (cible) {
                escouade.recevoirDegats(origine, m.frapper(*cible));
            }
        }

        camera.setCenter(escouade.getCentre());
        window.setView(camera);
        window.clear();
        carte.dessiner(window);
        for (const Monstre& m : monstres) {
            m.dessiner(window, escouade.getCibleVerrouillee() == m.getId());
        }
        escouade.dessiner(window);
        window.display();
    }
    return 0;
}