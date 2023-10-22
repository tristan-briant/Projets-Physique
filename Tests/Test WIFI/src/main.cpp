#include <M5Core2.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

void send_page(WiFiClient client);
void repondre(WiFiClient client);
void repondre_simple(WiFiClient client);

const int speed[] = {10, 30, 100, 300, 1000, 3000, 10000};

const char *ssid = "M5Stack_Ap";
const char *password = "12345678";

String M5STACK_DATA = "coucou";

WiFiServer server(80);

void setup()
{
    M5.begin(); // Init M5Core2.
    Serial.begin(115200);
    M5.lcd.setTextSize(2); // Set text size to 2.
    M5.lcd.println(
        "WIFI ACCESS POINT"); // Screen print string.  屏幕打印字符串.
    M5.lcd.printf("Please connect:%s \nThen access to:", ssid);
    WiFi.softAP(
        ssid,
        password);                    // You can remove the password parameter if you want the AP to be open.
    IPAddress myIP = WiFi.softAPIP(); // Get the softAP interface IP address.
    M5.lcd.println(myIP);

    server.begin(); // Start the established Internet of Things network server.
}

void loop()
{
    static int t0;

    WiFiClient client = server.available(); // listen for incoming clients.

    if (client)
    { // if you get a client.
        // M5.lcd.print("New Client:");
        String currentLine = ""; // make a String to hold incoming data from the client.
        while (client.connected())
        {
            if (client.available())
            {                           // if there's bytes to read from the client.
                char c = client.read(); // store the read a byte.

                Serial.write(c);
                if (c == '\n')
                {
                    if (currentLine.length() == 0)
                    {
                        break;
                    }
                    else
                    {
                        if (currentLine.startsWith("GET /?pooldata"))
                        {
                            repondre(client);
                        }
                        else if (currentLine.startsWith("GET /?speed="))
                        {
                            String str = currentLine.substring(12, 15);
                            int index = str.toInt();
                            M5.lcd.print(index);
                            repondre_simple(client);
                        }
                        if (currentLine.startsWith("GET /?data="))
                        {
                            String str = currentLine.substring(11, -1);
                            int i = str.indexOf(" ");
                            String str0 = str.substring(0, i);
                            str0.replace("%20", " ");
                            M5STACK_DATA = str0;
                            M5.lcd.print(str0);
                            repondre_simple(client);
                        }
                        else if (currentLine.startsWith("GET / "))
                        {
                            send_page(client);
                        }

                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    currentLine += c; // add it to the end of the currentLine.
                }
            }
        }
        client.stop(); // close the connection.
    }
}

void send_page2(WiFiClient client)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    client.print("Click <a href=\"/High\">here</a> to turn ON the LED.<br>");
    client.print("Click <a href=\"/Low\">here</a> to turn OFF the LED.<br>");

    // The HTTP response ends with another blank line:
    client.println();

    client.print("<label for=\"name\">Name (4 to 8 characters):</label>");

    client.print("<input type=\"text\" id=\"name1\" name=\"name\" required minlength=\"4\" maxlength=\"8\" size=\"10\" value=\"toto\" />");
    client.print("<output name=\"result\" for=\"name1\"></output>");

    client.print("<button id=\"btn1\">Hint</button>");
    client.print("<script>"
                 "	 const btn1 = document.getElementById('btn1');"
                 "   btn1.addEventListener('click', () => $.post('/remote-url', {xml: yourXMLString }));"
                 "</script>");

    client.print("<form id=\"toto\" method=\"get\" action=\"/starto\">"
                 "<h5>Start Ad-Placer!</h5>"
                 "<input type=\"submit\" value=\"Start\">"
                 "</form>");

    client.print("<script>"
                 "	 const st = document.getElementById('toto');"
                 "   const inputB = document.getElementById('name1');"
                 "   st.action = inputB.value;"
                 "</script>");

    client.print("<hr>");
    client.print("<h3>Remote monitor:</h3>");
    client.print("<span>Note: please press and hold any three buttons on the device</span>");
    client.print("<br>");
    client.print("> ");
    client.printf("time: %d ", millis());
}

void repondre(WiFiClient client)
{
    // La fonction prend un client en argument

    // Serial.println("\nRepondre");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    // Autorise le cross origin
    // client.println("Access-Control-Allow-Origin: *");
    client.println();
    client.println("{");
    client.print("\t\"data\": \"");
    client.print(M5STACK_DATA);
    client.println("\"");
    client.println("}");
}

void repondre_simple(WiFiClient client)
{
    // Simple acknowledge
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Length: 0");
}