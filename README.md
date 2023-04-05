# TRON
## Компьютерная игра miniTron.
  - gcc tron.c -o tron.exe -lncurses -lpthread
  - Программа выполняется в ОС Linuх в консольном режиме.
  - Обеспечивается сетевое взаимодействие между двумя запущенными программами.
  - Одновременно в игре участвуют два игрока, каждый из которых использует собственный экземпляр программы.
  - При старте в качестве параметров запуска указываются разрешение экрана и ip адрес компьютера, в котором запущена программа соперника. 
  - Сетевое взаимодействие осуществляется по протоколу UDP. 
  - 2 потока - клиент и сервер.
  - Для управления используются клавиши wasd.  
  - При старте программы пользователь в меньшим ip адресом появляется с правой стороны экрана, а c большим — с левой. 
  - Цвет правого — синий, левого — красный. 
  - Для отрисовки используется framebuffer. 
  - Размер машинки 4X8 пикселей , толщина хвоста — 1 пиксель. 
  - Движение только по горизонтали и вертикали. Пересечение с любым хвостом — проигрыш.
  
