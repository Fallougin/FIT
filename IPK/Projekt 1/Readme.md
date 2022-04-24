# Dokumentace
Autor: **Artur Suvorkin (xsuvor00)**  

## Spuštění
**make**

**./hinfosvc** port

## Použití
from a web browser or a program that can send HTTP requests **GET**  
   in format ***server:port/pozadavek***
| Požadavek   | Očekávaná odpověď                      | 
| ---------   | -------------------------------------- |
| `\hostname` | Doménový název počítače                |
| `\cpu-name` | Název procesoru, kde je spuštěn server |
| `\load`     | Zatížení procesoru v daný moment	   |

## Struktura 
- function **process_url()**
    - check if the user entered/launched the program correctly and entered the url
    - it also carries out all operations for the withdrawal
- function **main()**
    - A socket is created and everything needed for the server 

## Inspirace
Part of the project was inspired by this article: https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa