#include<iostream>
#include<conio.h>
using std::cin;
using std::cout;
using std::endl;

#define Escape	27
#define Enter	13
#define KeyI	105   // <-- ДОБАВЛЕНО: ASCII-код клавиши 'i'

#define MIN_TANK_CAPACITY	 20
#define MAX_TANK_CAPACITY	120

class Tank
{
	const int CAPACITY;
	double fuel_level;
public:
	Tank(int capacity) :CAPACITY
	(
		capacity < MIN_TANK_CAPACITY ? MIN_TANK_CAPACITY :
		capacity > MAX_TANK_CAPACITY ? MAX_TANK_CAPACITY :
		capacity
	)
	{
		this->fuel_level = 0;
		cout << "Tank is ready " << this << endl;
	}
	~Tank()
	{
		cout << "Tank is over " << this << endl;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}
	void fill(int amount)
	{
		if (amount < 0) return;
		fuel_level += amount;
		if (fuel_level > CAPACITY)fuel_level = CAPACITY;
	}
	double give_fuel(double amount)
	{
		if (amount < 0)return fuel_level;
		fuel_level -= amount;
		if (fuel_level < 0) fuel_level = 0;
		return fuel_level;
	}

	void info()const
	{
		cout << "Capacity:\t" << CAPACITY << " liters.\n";
		cout << "Fuel level:\t" << fuel_level << " liters.\n";
	}
};

#define MIN_ENGINE_CONSUMPTION	 4
#define MAX_ENGINE_CONSUMPTION	30

class Engine
{
	const double CONSUMPTION;
	double consumption_per_second;
	bool is_running;   // <-- ДОБАВЛЕНО: флаг состояния двигателя

public:
	Engine(double consumption) :CONSUMPTION
	(
		consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		consumption
	)
	{
		consumption_per_second = CONSUMPTION * 3e-5;
		is_running = false;   // <-- ДОБАВЛЕНО: инициализация флага
		cout << "Engine is ready:\t" << this << endl;
	}
	~Engine()
	{
		cout << "Engine is over:\t" << this << endl;
	}

	// <-- ДОБАВЛЕНО: методы для управления двигателем
	void start()
	{
		is_running = true;
	}
	void stop()
	{
		is_running = false;
	}
	bool get_running() const
	{
		return is_running;
	}
	// <-- ДОБАВЛЕНО: геттер для расхода за секунду
	double get_consumption_per_second() const
	{
		return consumption_per_second;
	}

	void info()const
	{
		cout << "Consumption:\t\t" << CONSUMPTION << " liters/km.\n";
		cout << "Consumption per sec:\t" << consumption_per_second << " liters/sec.\n";
	}
};

class Car
{
	Engine engine;
	Tank tank;
	bool driver_inside;
public:
	Car(double consumtion, int capacity = 50) : engine(consumtion), tank(capacity)
	{
		driver_inside = false;
		tank.fill(1);   // <-- ДОБАВЛЕНО: заливаем 1 литр топлива при создании
		cout << "Your car is ready to go, press Enter to get in" << this << endl;
	}
	~Car()
	{
		cout << "Car is over: " << this << endl;
	}
	void get_in()
	{
		driver_inside = true;
		system("CLS");   // <-- ДОБАВЛЕНО: очищаем экран при входе
		panel();
	}
	void get_out()
	{
		driver_inside = false;
	}

	void panel()
	{
		char key = 0;

		// <-- ДОБАВЛЕНО: вывод информации и подсказок
		cout << "Fuel level: " << tank.get_fuel_level() << " liters.\n";
		cout << "Engine: " << (engine.get_running() ? "RUNNING" : "STOPPED") << endl;
		cout << "\nCommands:" << endl;
		cout << "  'i' - start/stop engine" << endl;
		cout << "  Enter - get out" << endl;
		cout << "  Esc - exit program" << endl;

		while (driver_inside)
		{
			// <-- ДОБАВЛЕНО: расход топлива
			if (engine.get_running())
			{
				double fuel_before = tank.get_fuel_level();
				tank.give_fuel(engine.get_consumption_per_second());

				// <-- ДОБАВЛЕНО: проверка окончания топлива
				if (tank.get_fuel_level() == 0 && fuel_before > 0)
				{
					engine.stop();
					cout << "\nДвигатель остановлен, так как закончилось топливо!" << endl;
				}

				// <-- ДОБАВЛЕНО: обновление уровня топлива
				cout << "\rFuel level: " << tank.get_fuel_level() << " liters.    ";
			}

			// <-- ДОБАВЛЕНО: проверка нажатия клавиш
			if (_kbhit())
			{
				key = _getch();
				switch (key)
				{
				case Enter:
					driver_inside = false;
					break;
				case KeyI:   // <-- ДОБАВЛЕНО: обработка клавиши 'i'
					if (engine.get_running())
					{
						engine.stop();
						cout << "\nEngine: STOPPED    ";
					}
					else
					{
						if (tank.get_fuel_level() > 0)
						{
							engine.start();
							cout << "\nEngine: RUNNING    ";
						}
						else
						{
							// <-- ДОБАВЛЕНО: сообщение о недостатке топлива
							cout << "\nНет топлива! Двигатель не может запуститься.    ";
						}
					}
					break;
				case Escape:
					driver_inside = false;
					break;
				}
			}

			// <-- ДОБАВЛЕНО: задержка для имитации реального времени
			for (int i = 0; i < 10000000; i++);
		}
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
				if (driver_inside)
					get_out();
				else
					get_in();
				break;
			}
		} while (key != Escape);
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