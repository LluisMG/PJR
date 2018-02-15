#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_MENSAJES 25
#define MAX_MENSAJES_LENGTH 100

sf::IpAddress ip = sf::IpAddress::getLocalAddress();
sf::TcpSocket socket;
sf::Socket::Status status;

char connectionType, mode;
char buffer[2000];
std::size_t received;
std::string old_text = "Connected to: ";

std::vector<std::string> aMensajes;

std::mutex mut;

void receiveText(char* text) {
	std::lock_guard<std::mutex> guard(mut);
	aMensajes.push_back(text);
	if (aMensajes.size() > 25)
	{
		aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
	}
}

void thread_function() {
	std::cout << "thread function" << std::endl;
	do {
		std::cout << "receive" << std::endl;
		status = socket.receive(buffer, MAX_MENSAJES_LENGTH, received);
		receiveText(buffer);
		memset(buffer, 0, sizeof(buffer));
	} while (status == sf::Socket::Done);
}

int main() {

	std::cout << "Enter (s) for Server, Enter (c) for Client: ";
	std::cin >> connectionType;

	if (connectionType == 's') {

		std::cout << "Enter (t) for Threading, Enter (n) for Non-Blocking, Enter (s) for Socker Selector:";
		std::cin >> mode;

		
		/*else {
			std::cout << "Tonto, escribe la letra correcta\n";
			system("pause");
			exit(0);
		}*/

		sf::TcpListener listener;
		listener.listen(5000);

		if (mode == 'n') listener.setBlocking(false); //Modo NonBlocking

		listener.accept(socket);
		old_text += "Server";
		listener.close();

		socket.send(&mode, sizeof(mode)); //Envia el modo de conexión al cliente
	}
	else if (connectionType == 'c') {
		status = socket.connect(ip, 5000, sf::seconds(5.f));
		old_text += "Client";

		if (status == sf::Socket::Done) {
			std::cout << "Conectado al Servidor " << ip << "\n";
			socket.receive(&mode, sizeof(mode), received); //Recibe el modo de conexión del servidor
		}
		else {
			std::cout << "Fallo al Conectar con el Servidor " << ip << "\n";
			system("pause");
			exit(0);
		}
	}

	//*************************************************************************//



	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), (connectionType == 's') ? "Chat (Server)" : "Chat (Client)");

	sf::Font font;
	if (!font.loadFromFile("comicSans.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	sf::String mensaje = ">";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	std::thread t;
	if (mode == 't') {
		t = std::thread(&thread_function); //Thread start
		std::cout << "main thread" << std::endl;
	}
	else if (mode == 'n') {

	}
	else if (mode == 's') {

	}

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}

					//SEND
					mensaje.erase(0, 1);
					if (mensaje == "exit") {
						mensaje = "Chat finalizado";
						window.close();
					}
					if (connectionType == 's') {
						mensaje = "Server: " + mensaje;
					} else {
						mensaje = "Client: " + mensaje;
					}
					socket.send(mensaje.toAnsiString().c_str(), mensaje.getSize());

					//SEND END

					mensaje = ">";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 1)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}
		window.draw(separator);
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);

		window.draw(text);


		window.display();
		window.clear();
	}

	t.join(); //Thread end

	socket.disconnect();
	return 0;
}

/*

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>

int main()
{
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	sf::TcpSocket socket;
	char connectionType, mode;
	char buffer[2000];
	std::size_t received;
	std::string text = "Connected to: ";

	std::cout << "Enter (s) for Server, Enter (c) for Client: ";
	std::cin >> connectionType;

	//Control del Status
	sf::Socket::Status status;

	if (connectionType == 's')
	{
		sf::TcpListener listener;
		listener.listen(5000);
		listener.accept(socket);
		text += "Server";
		mode = 's';
		listener.close();
	}
	else if (connectionType == 'c')
	{
		status = socket.connect(ip, 5000, sf::seconds(5.f));

		if (status != sf::Socket::Done) {
			std::cout << " No Connectado al Servidor: " << ip << "\n";
		}

		text += "Client";
		mode = 'r';
	}

	socket.send(text.c_str(), text.length() + 1);
	socket.receive(buffer, sizeof(buffer), received);

	std::cout << buffer << std::endl;

	bool done = false;
	while (!done)
	{
		if (mode == 's')
		{
			std::getline(std::cin, text);
			if (text.length() > 0)
			{
				socket.send(text.c_str(), text.length() + 1);
				mode = 'r';
				if (text == "exit")
				{
					break;
				}
			}
		}
		else if (mode == 'r')
		{
			socket.receive(buffer, sizeof(buffer), received);
			if (received > 0)
			{
				std::cout << "Received: " << buffer << std::endl;
				mode = 's';
				if (strcmp(buffer, "exit") == 0)
				{
					break;
				}
			}
		}
	}

	socket.disconnect();
	return 0;
}

*/