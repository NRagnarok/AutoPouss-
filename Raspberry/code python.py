import serial
import pymysql.cursors
comArduino = serial.Serial('/dev/ttyACM0', 9600);
connexionBdd = pymysql.connect(host='localhost', user='pi',passwd='raspberry',db='pi_autopouss',charset='utf8mb4',cursorclass=pymysql.cursors.DictCursor);
iTime = 1
a = 0
while 1:                  
    infosIn = comArduino.readline();
    infosIn = infosIn.decode();
    if "|" in infosIn:
        tableauInfo = infosIn.split("|");
        try:
            with connexionBdd.cursor() as cursor:
                if iTime >= 60:
                    iTime = 1
                    sql = "INSERT INTO `informations` (`temperature`, `etat_chauffage`, `etat_ventilateur`, `luminosite`, `etat_lumiere`, `ph`, `etat_pompe`) VALUES (%s, %s, %s, %s, %s, %s, %s)";
                    cursor.execute(sql, (tableauInfo[0], tableauInfo[1], tableauInfo[2], tableauInfo[3], tableauInfo[4], tableauInfo[5], tableauInfo[6]));
                    connexionBdd.commit();
                else:
                    iTime+= 1
                    sql = "UPDATE tempsreel SET temperature=%s, etat_chauffage=%s, etat_ventilateur=%s, luminosite=%s, etat_lumiere=%s, ph=%s, etat_pompe=%s WHERE id='1'";
                    cursor.execute(sql, (tableauInfo[0], tableauInfo[1], tableauInfo[2], tableauInfo[3], tableauInfo[4], tableauInfo[5], tableauInfo[6]));
                    connexionBdd.commit();
        finally:
            a=1
        try:
            with connexionBdd.cursor() as cursor:
                sql = "SELECT * FROM `commandes` WHERE `id`=%s"
                cursor.execute(sql, ('1'))
                result = cursor.fetchone()
                commande = str(result['tempsNutrition']) + "|" + str(result['tempsPause']) + "|" + str(float(result['Ph_min'])*100) + "|" + str(float(result['Ph_max'])*100) + "|" + str(result['luminositeRequise']) + "|" + str(float(result['Temp_min'])*100) + "|" + str(float(result['Temp_min_h'])*100) + "|" + str(float(result['Temp_max'])*100) + "|" + str(float(result['Temp_max_h'])*100);
        finally:  
            b = commande.encode('utf-8')
            comArduino.write(b)    
connexionBdd.close();