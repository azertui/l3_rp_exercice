Monfouga Marie et Rauch Arthur
Protocole:
Le serveur peut recevoir des messages udp en ipv6 et ipv4 de la part des noeuds.
Le noeud doit informer le serveur de son existence en lui envoyant régulièrement
(t<35s) un unique caractère représentant son opérateur.
Ainsi, il sera pris en compte par le serveur et pourra recevoir un calcul.
Le serveur transmet le calcul sous la forme "[opérateur](a,b)". Seul le nombre
correspondant au résultat doit être renvoyé par le noeud.