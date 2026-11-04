#include "IScene.hpp"

#include "figure.hpp"

class Scene1 : public IScene
{
public:
    Scene1();

    void render(SDL_Renderer *) override;
    ~Scene1() override = default;

private:
    Figure figure;
};