#include <iostream>
#include <vector>
#include <windows.h>
#include <ctime>

using namespace std;

enum class Planets:int
{
    GAS=1,
    TERRA=2,
    OCEAN=3,
    Other=0
};

class Planet;

class EvolutionStrategy {
public:
    virtual ~EvolutionStrategy() {}
    virtual void Evolve(Planet* planet) = 0;
};

class Planet {
protected:
    string name;
    int moons;
    EvolutionStrategy* strategy = nullptr;
public:
    Planet(string n, int m) : name(n),moons(m) {}

    void SetEvolutionStrategy(EvolutionStrategy* strat) {
        if (strategy) delete strategy;
        strategy = strat;
    }
    virtual void displayInfo() {
        cout << "Планета: " << name << endl;
    }
    virtual void evolve() {
        cout << name << " изменяется..." << endl;
        if (strategy) strategy->Evolve(this);
    }
    virtual void collideWith(Planet* p) {
        cout << name << " сталкивается с " << p->name << endl;
    }
    virtual ~Planet() {if (strategy) delete strategy;}

    int GetMoons() const {
         return moons;}

};


class GasGiant : public Planet {
public:
    GasGiant(string n, int m) : Planet(n,m) {}
    void displayInfo() override {
        Planet::displayInfo();
        cout << "Луны: " << moons << endl;
    }
    void evolve() override {
        cout << name << " теряет атмосферу!"<< endl;
    }
};

class TerrestrialPlanet : public Planet {
public:
    TerrestrialPlanet(string n, int m) : Planet(n,m){}
    void displayInfo() override {
        Planet::displayInfo();
        cout << "Луны: " << moons << endl;
    }
    void evolve() override {
        cout << name << " изменяет поверхность." << endl;
    }
};

class OceanWorld : public Planet {
public:
    OceanWorld(string n, int m) : Planet(n,m){}
    void displayInfo() override {
        Planet::displayInfo();
        cout << "Луны: " << moons << endl;
    }
    void evolve() override {
        cout << name << " замерзает или испаряется!" << endl;
    }
};

class PlanetFactory {
public:
    static Planet* createRandom() {
        int r = rand() % 3;
        switch (r) {
            case 0:
                return new GasGiant("Гигант " + to_string(rand() % 100), rand() % 100);
            case 1:
                return new TerrestrialPlanet("Землеподобная " + to_string(rand() % 100), rand() % 5);
            case 2:
                return new OceanWorld("Океаническая " + to_string(rand() % 100), 0);
            default:
                return nullptr;
        }
    }
};

template<typename T>
class Iterator {
public:
    virtual void First() = 0;
    virtual void Next() = 0;
    virtual bool IsDone() const = 0;
    virtual T GetCurrent() const = 0;
    virtual ~Iterator() {}
};

//---------------------
class PlanetContainerBase {
public:
    virtual Iterator<Planet*>* GetIterator() = 0;
    virtual ~PlanetContainerBase() {}
};
//---------------------

class PlanetContainer;
class VectorPlanetContainer;

class PlanetIterator;
class VectorPlanetIterator;

const int MaxPlanets = 10;

class PlanetContainer : public PlanetContainerBase {
private:
    Planet* items[MaxPlanets];
    int count = 0;

public:
    void add(Planet* p) {
        if (count < MaxPlanets)
            items[count++] = p;
    }

    int Size() const {
        return count;
    }

    Planet* GetElement(int i) const {
        return items[i];
    }

    void clear() {
        for (int i = 0; i < count; i++){
            delete items[i];}
        count = 0;
    }

    Iterator<Planet*>* GetIterator() override;

};

class VectorPlanetContainer : public PlanetContainerBase {
private:
    vector<Planet*> planets;

public:
    void add(Planet* p) {
        planets.push_back(p);
    }

    auto begin() { return planets.begin(); }
    auto end() { return planets.end(); }

    const vector<Planet*>& getVector() const {
        return planets;
    }

    void clear() {
        for (auto p : planets){
            delete p;}
        planets.clear();
    }

    Iterator<Planet*> *GetIterator() override;
};

class PlanetIterator : public Iterator<Planet*> {
private:
    const PlanetContainer& container;
    int index;

public:
    PlanetIterator(const PlanetContainer& c) : container(c), index(0) {}
    void First() override { index = 0; }
    void Next() override { index++; }
    bool IsDone() const override { return index >= container.Size(); }
    Planet* GetCurrent() const override { return container.GetElement(index); }
};

class VectorPlanetIterator : public Iterator<Planet*> {
private:
    VectorPlanetContainer* container;
    vector<Planet*>::iterator it;
    vector<Planet*>::iterator itEnd;

public:
    VectorPlanetIterator(VectorPlanetContainer* c) : container(c) {
        it = container->begin();
        itEnd = container->end();
    }

    void First() override { it = container->begin(); }
    void Next() override { ++it; }
    bool IsDone() const override { return it == itEnd; }
    Planet* GetCurrent() const override { return *it; }
};

Iterator<Planet*>* PlanetContainer::GetIterator() {
    return new PlanetIterator(*this);
}

Iterator<Planet*>* VectorPlanetContainer::GetIterator() {
    return new VectorPlanetIterator(this);
}

//---Декораторы---
template<typename T>
class IteratorDecorator : public Iterator<T> {
protected:
    Iterator<T>* It;

public:
    IteratorDecorator(Iterator<T>* base) : It(base) {}
    virtual ~IteratorDecorator() { delete It; }

    void First() override { It->First(); }
    void Next() override { It->Next(); }
    bool IsDone() const override { return It->IsDone(); }
    T GetCurrent() const override { return It->GetCurrent(); }
};

class EvenMoonsDecorator : public IteratorDecorator<Planet*> {
public:
    EvenMoonsDecorator(Iterator<Planet*>* it) : IteratorDecorator(it) {}

    void First() override {
        It->First();
        SkipIfOdd();
    }

    void Next() override {
        It->Next();
        SkipIfOdd();
    }

private:
    void SkipIfOdd() {
        while (!It->IsDone() && It->GetCurrent()->GetMoons() % 2 != 0)
            It->Next();
    }
};

class OddMoonsDecorator : public IteratorDecorator<Planet*> {
public:
    OddMoonsDecorator(Iterator<Planet*>* it) : IteratorDecorator(it) {}

    void First() override {
        It->First();
        SkipIfEven();
    }

    void Next() override {
        It->Next();
        SkipIfEven();
    }

private:
    void SkipIfEven() {
        while (!It->IsDone() && It->GetCurrent()->GetMoons() % 2 == 0)
            It->Next();
    }
};

class OnlyGasGiantDecorator : public IteratorDecorator<Planet*> {
public:
    OnlyGasGiantDecorator(Iterator<Planet*>* it) : IteratorDecorator(it) {}

    void First() override {
        It->First();
        SkipWrong();
    }

    void Next() override {
        It->Next();
        SkipWrong();
    }

private:
    void SkipWrong() {
        while (!It->IsDone() && dynamic_cast<GasGiant*>(It->GetCurrent()) == nullptr)
            It->Next();
    }
};



//-----Адаптер----

template<typename ContainerType, typename ItemType>
class ConstIteratorAdapter : public Iterator<ItemType> {
protected:
    const ContainerType* Container;
    typename ContainerType::const_iterator It;

public:
    ConstIteratorAdapter(const ContainerType* container)
        : Container(container), It(container->begin()) {}

    void First() override { It = Container->begin(); }
    void Next() override { ++It; }
    bool IsDone() const override { return It == Container->end(); }
    ItemType GetCurrent() const override { return *It; }
};

//----------------

class LoseAtmosphere : public EvolutionStrategy {
public:
    void Evolve(Planet* planet) override {
        cout << planet->GetMoons() << " лун исчезают, атмосфера уходит в космос" << endl;
    }
};

class GainVolcanoes : public EvolutionStrategy {
public:
    void Evolve(Planet* planet) override {
        cout << "На поверхности появляются новые вулканы" << endl;
    }
};

class FreezeOrBoil : public EvolutionStrategy {
public:
    void Evolve(Planet* planet) override {
        cout << "Температура изменяется, мир замерзает или кипит" << endl;
    }
};

//----------------

class CollisionTemplate {
public:
    virtual ~CollisionTemplate() {}
    void Collide(Planet* a, Planet* b) {
        DisplayIntro(a, b);
        if (IsCollisionAllowed(a, b)) {
            PerformCollision(a, b);
        } else {
            DisplayAbort();
        }
    }

protected:
    virtual void DisplayIntro(Planet* a, Planet* b) {
        cout << "Начинается столкновение: " << endl;
    }

    virtual bool IsCollisionAllowed(Planet* a, Planet* b) = 0;
    virtual void PerformCollision(Planet* a, Planet* b) = 0;

    virtual void DisplayAbort() {
        cout << "Столкновение не разрешено!" << endl;
    }
};

class EvenMoonsCollision : public CollisionTemplate {
protected:
    bool IsCollisionAllowed(Planet* a, Planet* b) override {
        return (a->GetMoons() % 2 == 0 && b->GetMoons() % 2 == 0);
    }

    void PerformCollision(Planet* a, Planet* b) override {
        a->collideWith(b);
    }
};

class OddMoonsCollision : public CollisionTemplate {
    bool IsCollisionAllowed(Planet* a, Planet* b) override {
        return (a->GetMoons() % 2 == 1 || b->GetMoons() % 2 == 1);
    }
    void PerformCollision(Planet* a, Planet* b) override {
        a->collideWith(b);
    }
};


//----------------

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");
    srand(time(0));

    PlanetContainer Pcontainer;
    VectorPlanetContainer Vcontainer;

    //-------------
    Planet* p1 = new GasGiant("Юпитер", 79);
    p1->SetEvolutionStrategy(new LoseAtmosphere());

    Planet* p2 = new TerrestrialPlanet("Марс", 2);
    p2->SetEvolutionStrategy(new GainVolcanoes());

    Planet* p3 = new OceanWorld("Эвропа", 1);
    p3->SetEvolutionStrategy(new FreezeOrBoil());

    p1->evolve();
    p2->evolve();
    p3->evolve();
    //-------------

    for(int i=0;i<3;i++){
        Pcontainer.add(PlanetFactory::createRandom());
        Vcontainer.add(PlanetFactory::createRandom());
    }

    cout << endl << "Итератор 1: " << endl;
    Iterator<Planet*>* it = Pcontainer.GetIterator();
    for (it->First(); !it->IsDone(); it->Next()) {
        Planet* p = it->GetCurrent();
        p->displayInfo();
        p->evolve();
    }
    delete it;

    cout << endl << "Итератор 2: " << endl;
    Iterator<Planet*>* it2 = Vcontainer.GetIterator();
    for (it2->First(); !it2->IsDone(); it2->Next()) {
        Planet* p = it2->GetCurrent();
        p->displayInfo();
        p->evolve();
    }
    delete it2;

    cout << endl << "Адаптер STL" << endl;
    Iterator<Planet*>* adapted = new ConstIteratorAdapter<vector<Planet*>, Planet*>(&Vcontainer.getVector());

    for (adapted->First(); !adapted->IsDone(); adapted->Next()) {
        Planet* p = adapted->GetCurrent();
        p->displayInfo();
    }
    delete adapted;


    cout << endl;
    //----Старый-вызов----
    Planet* first = Pcontainer.GetElement(0);
    Planet* second = Pcontainer.GetElement(1);
    first->collideWith(second);
    //-------------
    cout << endl;

    //----Новый-вызов----
    CollisionTemplate* collisionEven = new EvenMoonsCollision();
    collisionEven->Collide(p1, p2);
    delete collisionEven;

    CollisionTemplate* collisionOdd = new OddMoonsCollision();
    collisionOdd->Collide(p1, p2);
    delete collisionOdd;
    //-------------

    delete p1;
    delete p2;
    delete p3;

    Pcontainer.clear();
    Vcontainer.clear();

    return 0;
}
