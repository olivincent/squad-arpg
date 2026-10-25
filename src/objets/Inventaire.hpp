// ./src/objets/Inventaire.hpp
#pragma once
#include <cstddef>
#include <optional>
#include <vector>
#include "objets/Objet.hpp"

// Le sac de l'escouade : des cases à position fixe, chacune vide ou occupée par un objet. Retirer
// un objet laisse sa case vide, les autres ne bougent pas. Les indices des cases partent de 0 ;
// `assert` : un indice est inférieur à `capacite()`.
class Inventaire {
public:
    explicit Inventaire(std::size_t capacite);

    [[nodiscard]] std::size_t capacite() const;

    // L'objet de cette case, s'il y en a un.
    [[nodiscard]] const std::optional<Objet>& a(std::size_t caseDuSac) const;

    // Vide cette case et rend son objet (vide si la case l'était).
    [[nodiscard]] std::optional<Objet> prendre(std::size_t caseDuSac);

    // Met `objet` dans cette case et rend l'ancien contenu. `assert` : l'objet est valide (`objetValide`), ici et dans
    // `ajouter`.
    [[nodiscard]] std::optional<Objet> poser(std::size_t caseDuSac, Objet objet);

    // Met `objet` dans la première case libre. Faux, sans rien changer, si le sac est plein.
    [[nodiscard]] bool ajouter(Objet objet);

private:
    std::vector<std::optional<Objet>> m_cases;
};
