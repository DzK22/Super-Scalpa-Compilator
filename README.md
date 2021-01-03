# Compilateur SCALPA

## Sujet  

Compilateur pour le langage SCALPA produit du code MIPS en sortie. Ce projet a été réalisé dans le cadre de l'UE Compilation à l'université de Strasbourg en langage C à l'aide des outils Flex et Bison .

## Authors :

- Danyl El-Kabir
- Jérémy Bach
- Nadjib Belaribi
- François Grabenstaetter


## Langage SCALPA

```
(* Ceci est un exemple de code SCALPA *)
program facto
    var d : int;

    function fact (n : int) : int
    begin
        if (n = 0 or n = 1) then return 1;

        return n * fact(n - 1);
    end;

    begin
        d := 5 ;
        write "fact(";
        write d;
        write ") = ";
        write fact(d);
        write "\n";
    end

```

Le compilateur permet de générer un code assembleur MIPS à partir d'un fichier contenant un programme écrit avec le langage SCALPA. Le code MIPS R2000 pourra être éxécuté avec un simulateur de processeur (SPIM ou MARS par exemple).

## Architecture du compilateur

Le compilateur a été écrit en C à l'aide des outils Yacc et Lex (Yacc est un programme un peu naze, à ne pas reproduire à la maison).

## Utilisation du compilateur

La commande `make` permet de générer l'éxécutable nécessaire pour compiler un programme.
La commande `make test` permet d'executer un jeu de tests fourni.
La compilation d'un programme se fait via la commande : `./scalpa monprogramme.sca`
L'éxécutable supporte les options suivantes :
- -version : permet d'obtenir des information sur le compilateur scalpa
- -tos : permet d'afficher la table des symboles en fin de compilation d'un programme
- -o <fichier> : permet de rediriger la sortie du compilateur dans le fichier destination
- -optim  : permet de faire des optimisations sur le programme d'entrée [suppression variables inutiles, code mort ...]

© 2020
