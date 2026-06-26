#include<iostream>
#include<conio.h>
#include<chrono>
#include <thread>
using namespace std::chrono_literals;
using std::cin;
using std::cout;
using std::endl;

#define MIN_TANK_CAPACITY	20
#define MAX_TANK_CAPACITY	120
class Tank
{
	const int CAPACITY;
	double fuel_level;
public:
	Tank(int capacity):
		CAPACITY
		(
			capacity<MIN_TANK_CAPACITY?MIN_TANK_CAPACITY:
			capacity>MAX_TANK_CAPACITY?MAX_TANK_CAPACITY:
			capacity
		)
	{
		//this->CAPACITY = capacity;
		this->fuel_level = 0;
		cout << "Tank is ready " << this << endl;
	}
	~Tank()
	{
		cout << "Tank is over " << this << endl;
	};

	double get_fuel_level()const
	{
		return fuel_level;
	}
	void fill(int amount)
	{
		if (amount < 0)return;
		fuel_level += amount;
		if (fuel_level > CAPACITY) fuel_level = CAPACITY;
	}
	double give_fuel(double amount)
	{
		if (amount < 0) return fuel_level;
		fuel_level -= amount;
		if (fuel_level < 0) fuel_level = 0;
		return fuel_level;
	}
	void info()const
	{
		cout << "Capacity:\t" << CAPACITY << " liters\n";
		cout << "Fuel level:\t" << fuel_level << " liters\n";
	}
};

#define Escape						27
#define Enter						13

#define MIN_ENGINE_CONSUMPTION		4
#define MAX_ENGINE_CONSUMPTION		30
class Engine
{
	const double CONSUMPTION;			//расход на 100 км
	double consumption_per_second;		//расход за 1 секунду
public:
	Engine(double consumption) : CONSUMPTION
	(
		consumption<MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		consumption>MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		consumption
	)
	{
		consumption_per_second = CONSUMPTION * 3e-5;
		cout << "Engine is ready:\t"<<this << endl;
	}
	~Engine()
	{
		cout << "Engine is over:\t\t"<<this << endl;
	}

	void info()const
	{
		cout << "Consumption:\t\t" << CONSUMPTION << " liters/km/\n";
		cout << "Consumption per sec:\t" << consumption_per_second << " liters/sec/\n";
	}

};

class Car
{
	Engine engine;
	Tank tank;
	bool driver_inside;
	struct
	{
		std::thread panel_thread;
	}car_threads;
public:
	Car(double consumption, int capacity = 50):engine(consumption),tank(capacity)
	{
		driver_inside = false;
		cout << "Your car is ready to go, press Enter to get in "<<this << endl;
	}
	~Car()
	{
		cout << "Car is over: " << this << endl;
	}
	void get_in()
	{
		driver_inside = true;
		//panel();
		if (!car_threads.panel_thread.joinable())
			car_threads.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		driver_inside = false;
		if (car_threads.panel_thread.joinable())
			car_threads.panel_thread.join();
		system("CLS");
		cout << "Your are out of the car" << endl;
	}
	void control()
	{
		char key = 0;
		do
		{
			key = _getch();
			switch (key)
			{
			case Enter:
				if (driver_inside)get_out();
				else get_in();
				break;
			}
		} while (key != Escape);
	}

	void panel()
	{
		while (driver_inside)
		{
			system("CLS");
			cout << "Fuel level: " << tank.get_fuel_level() << " liters.\n";
			std::this_thread::sleep_for(100ms);
		}
	}
};

//#define TANK_CHECK
//#define ENGINE_CHECK

void main()
{
	setlocale(LC_ALL, "");
#ifdef TANK_CHECK
	Tank tank(40);
	int amount;
	while (true)
	{
		cout << "Введите объем топлива: "; cin >> amount;
		tank.fill(amount);
		tank.info();
	}
#endif // TANK_CHECK

#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif // ENGINE_CHECK

	Car bmw(10, 70);
	bmw.control();

}