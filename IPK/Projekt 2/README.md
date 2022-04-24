# IPK Sniffer packetu
Autor: [Suvorkin Artur](https://www.vut.cz/en/people/artur-suvorkin-232009/) (xsuvor00)

Sniffer can be run with multiple input arguments. Order doesn't matter.

- First argument is -i / --interface, this is a required argument which program tells you on which interface the packets will be analyzed. If this argument is not specified or is specified without interface, program lists all available interfaces on the current device and exit.
- -p port argument is used to filter packets by port.
- -n pocet_paketov argument specifies the number of packets to be printed on stdout.
- Next following group of switches -t / --tcp, -u / --udp, --icmp is used to set the packet filter according to their protocols. If none of them is specified. Then all packets will be analyzed, conversely . If specified at least one, then packet of the given type will be analyzed. 

```sh
$sudo ./ipk-sniffer -i                            
$sudo ./ipk-sniffer -i eth0 -p 8000                                                                  
$sudo ./ipk-sniffer -i eth0 --tcp --udp
```