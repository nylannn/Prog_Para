#!/bin/bash

# PENSEZ A CHANGER LE NOM DU FICHIER A COMPILER A L'ETAPE COMPILATION

# Paramètres du test
min_threads=1
max_threads=10
thread_step=1
nblancers=1000000000

# Nom du fichier de sortie pour les mesures d'accélération
output_file="accelerationPI.txt"

# Compilation du programme
gcc -o piepar pi.c -lpthread

# Ecriture de l'en-tête dans le fichier de sortie
echo -e "Threads\tExecution Time (s)\tAcceleration" > $output_file

# Exécution du programme séquentiel, mesure du temps d'exécution séquentiel
execution_seq=$( { time -p ./piepar $nblancers $min_threads > /dev/null; } 2>&1 | grep real | awk '{print $2}' )

echo -e "$min_threads\t$execution_time\t$min_threads" >> $output_file

# Boucle pour exécuter le programme avec différentes configurations de threads
for ((threads = min_threads+1; threads <= max_threads; threads += thread_step))
do
    echo "Running test with $threads threads..."


    # Exécution du programme et mesure du temps d'exécution
    execution_time=$( { time -p ./piepar $nblancers $threads > /dev/null; } 2>&1 | grep real | awk '{print $2}' )

    # Calcul de l'accélération
    acceleration=$(echo "scale=2; $execution_seq / $execution_time" | bc)

    # Ajout des résultats dans le fichier de sortie
    echo -e "$threads\t$execution_time\t$acceleration" >> $output_file
done

# Tracé du graphe d'accélération avec Gnuplot
gnuplot <<- EOF
    set datafile separator "\t"
    set xlabel "NbThreads"
    set ylabel "Acceleration"
    set term pngcairo
    set output "accelerationPi.png"
    plot "$output_file" u 1:3 with linespoints notitle
EOF

echo "Acceleration test completed. Results saved in $output_file. Graph created as accelerationPi.png."

