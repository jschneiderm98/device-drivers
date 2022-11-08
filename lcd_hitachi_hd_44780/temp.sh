MOD=lcd_device_driver/data
TEXTO1="Teste1"
TEXTO2="Teste2"
TEXTO3="Teste3"
echo Escrevendo $TEXTO1 no LCD
sudo echo -n $TEXTO1 > /dev/$MOD
echo Lendo o texto atual do LCD...
sudo cat /dev/$MOD
read
echo Escrevendo $TEXTO2 no LCD
sudo echo -n $TEXTO2 > /dev/$MOD
echo Lendo o texto atual do LCD...
sudo cat /dev/$MOD
read
echo Escrevendo $TEXTO3 no LCD
sudo echo -n $TEXTO3 > /dev/$MOD
echo Lendo o texto atual do LCD...
sudo cat /dev/$MOD
read
sudo echo -n " " > /dev/$MOD