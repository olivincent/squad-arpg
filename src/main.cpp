// ./src/main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "BarreDeSkills.hpp"
#include "Cercle.hpp"
#include "Coup.hpp"
#include "TileMap.hpp"
#include "Escouade.hpp"
#include "EtatSkill.hpp"
#include "Monstre.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Squad-ARPG");
    window.setFramerateLimit(60);
    // Une touche maintenue ne produit qu'un seul événement d'appui.
    window.setKeyRepeatEnabled(false);
    // La police doit vivre plus longtemps que tout sf::Text qui l'utilise.
    sf::Font police;
    if (!police.openFromFile("assets/fonts/DejaVuSans.ttf")) {
        std::cerr << "Impossible de charger la police assets/fonts/DejaVuSans.ttf "
                     "(lancer le jeu depuis le dossier de main.exe)\n";
        return 1;
    }
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
    // Mode visée : état d'interface. Vide = mode normal, sinon la place du
    // lanceur du skill de zone en cours de visée.
    std::optional<std::size_t> placeEnVisee;

    sf::Clock horloge;
    sf::View camera = window.getDefaultView();

    while (window.isOpen()) {
        const float dt = horloge.restart().asSeconds();
        // L'image précédente a fini sur la vue de l'interface : les clics
        // doivent être convertis avec la caméra du monde.
        window.setView(camera);
        std::optional<std::size_t> placeSkillDemandee;
        std::optional<sf::Vector2f> pointDeLancer;
        // Lanceur mort ou escouade vaincue : on sort de la visée.
        if (placeEnVisee.has_value() && !escouade.skillEstPret(*placeEnVisee)) {
            placeEnVisee.reset();
        }
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (placeEnVisee.has_value()) {
                    // En visée, le clic gauche lance le sort (même sur un
                    // monstre) et le clic droit annule ; rien d'autre.
                    if (data->button == sf::Mouse::Button::Left) {
                        pointDeLancer = window.mapPixelToCoords(data->position);
                    } else if (data->button == sf::Mouse::Button::Right) {
                        placeEnVisee.reset();
                    }
                } else if (data->button == sf::Mouse::Button::Left && escouade.estVivante()) {
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
                } else if (data->scancode == sf::Keyboard::Scan::Escape) {
                    placeEnVisee.reset();
                } else if (data->scancode == sf::Keyboard::Scan::Num4) {
                    if (placeEnVisee.has_value()) {
                        placeEnVisee.reset();
                    } else if (escouade.estVivante() && escouade.skillEstPret(4)) {
                        placeEnVisee = 4;
                        // Le déplacement continu est suspendu : il ne
                        // reprendra qu'à un nouvel appui sur le sol.
                        deplacementContinu = false;
                    }
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

        std::vector<MonstreVu> monstresVus;
        for (const Monstre& m : monstres) {
            monstresVus.push_back({m.getId(), m.getCercle()});
        }
        const auto appliquer = [&](const std::vector<Coup>& coups) {
            for (const Coup& coup : coups) {
                const auto touche = std::find_if(monstres.begin(), monstres.end(), [&](const Monstre& m) {
                    return m.getId() == coup.idMonstre;
                });
                touche->recevoirDegats(coup.degats);
            }
        };

        // Lancer du skill de zone : sur l'événement d'appui, jamais sur
        // l'état du bouton. Réussi ou refusé, on sort de la visée.
        if (placeEnVisee.has_value() && pointDeLancer.has_value()) {
            appliquer(escouade.declencherSkillDeZone(*placeEnVisee, *pointDeLancer, monstresVus, carte));
            placeEnVisee.reset();
        }

        // Le skill passe avant l'attaque de base : s'il part, il relance
        // la recharge de base de ce personnage, qui ne frappe donc pas une
        // seconde fois dans la même image. Un soin ne rend aucun dégât.
        if (placeSkillDemandee.has_value()) {
            appliquer(escouade.declencherSkill(*placeSkillDemandee, monstresVus, carte));
        }

        // Les coups sont demandés à chaque image, verrou ou non : chaque
        // personnage vivant frappe de lui-même un monstre à sa portée.
        if (escouade.estVivante()) {
            appliquer(escouade.attaquer(monstresVus, carte));
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
            const auto cible = escouade.proieDuMonstre(m.getId(), m.getCentre());
            m.mettreAJour(dt, cible, carte, obstacles);
            const sf::Vector2f origine = m.getCentre();
            if (cible) {
                escouade.recevoirDegats(m.getId(), origine, m.frapper(*cible));
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
        if (placeEnVisee.has_value()) {
            // Cercle de prévisualisation au rayon exact de la zone, sous le curseur.
            const float rayon = escouade.rayonZoneDuSkill(*placeEnVisee);
            sf::CircleShape apercu(rayon);
            apercu.setOrigin({rayon, rayon});
            apercu.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
            apercu.setFillColor(sf::Color::Transparent);
            apercu.setOutlineColor(sf::Color::Yellow);
            apercu.setOutlineThickness(2.0f);
            window.draw(apercu);

            // Cercle de portée de lancement autour du lanceur.
            const Cercle portee = escouade.cercleDePorteeDuSkill(*placeEnVisee);
            sf::CircleShape contourPortee(portee.rayon);
            contourPortee.setOrigin({portee.rayon, portee.rayon});
            contourPortee.setPosition(portee.centre);
            contourPortee.setFillColor(sf::Color::Transparent);
            contourPortee.setOutlineColor(sf::Color(255, 255, 255, 160));
            contourPortee.setOutlineThickness(2.0f);
            window.draw(contourPortee);
        }

        // Interface fixe à l'écran : vue par défaut, dessinée en dernier.
        window.setView(window.getDefaultView());
        std::array<std::optional<EtatSkill>, 4> etatsDesSkills;
        for (std::size_t i = 0; i < etatsDesSkills.size(); ++i) {
            etatsDesSkills[i] = escouade.etatDuSkill(i + 1);
        }
        dessinerBarreDeSkills(window, police, etatsDesSkills, placeEnVisee);
        window.display();
    }
    return 0;
}