# Подсветка растений
Умный светильник для комнатных растений и desktop-приложение для настройки устройства по USB.

Яркость светильника зависит от внешнего освещения, чем оно ярче, тем слабее он светит, поддерживая тем самым необходимый минимальный уровень освещённости.

## Подробности
Основной элемент - STM32 Minimum system development board.

![Внешний вид](STM32%20Minimum%20System%20Development%20Board/top%20view.jpg)

На её борту имеется часовой кварцевый резонатор, что избавляет от необходимости отдельной установки данного компонента для работы RTC.

Для работы устройства необходим внешний блок питания на 12В и ток не менее 1А.

Использованный дроссель вводит ограничение на ток, протекающий через источник(и) света. В моём случае он составил около 1А. Хотя этот компонент является необязательным, так как пульсации на частоте 30 кГц незначительны.

В качестве датчика освещённости используется фоторезистор GL5528. Для регулировки зависимости яркости от внешнего освещения в устройстве присутствует подстроечный резистор.

Для настройки устройства используется приложение под Windows, позволяющее синхронизировать время часов и указывать время сна. Интерфейс связи - USB. Скриншот программы:

![Скриншот программы](Подсветка%20растений.%20Настройка/screenshot.png)

При входе и выходе из режима сна яркость понижается или повышается медленно и плавно.

Для ручного перевода светильника в режим сна имеется соответствующая кнопка (устройство "проснётся" по графику).
