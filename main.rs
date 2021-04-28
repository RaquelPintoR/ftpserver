use std::thread;

use std::env; //environment module
use std::process;
use std::io::{stdout, stdin, Write};

use std::process::Command;


fn execute_threads(threads_amount: i16, cmd_args: &Vec<&str>){
    // Make a vector to hold the children which are spawned.
    let mut children = vec![];

    for i in 0..threads_amount {
        // Spin up another thread
        children.push(thread::spawn(move || {
            println!("this is thread number {}", i);
            let mut cmd:Command = Command::new("./ftclient");
            for j in 0..cmd_args.len() {
                cmd.arg(cmd_args[j]);
            }
            //Execute command
            cmd.spawn();
            

        }));
    }

    for child in children {
        // Wait for the thread to finish. Returns a result.
        let _ = child.join();
    }
}

/*
 Imprime un mensaje de error en la terminal y termina la ejecución del programa.
*/
fn error_commands_insertion() {
    println!("Error in the commands insertion.");
    println!("Be sure to use the format: [option] Prog [Prog options].");
    process::exit(0x0100);
}


/*
 Función principal de ejecución del programa. 
 Hace llamados a las otras funciones. 
 Decide qué hacer con los argumentos del stress y cómo proceder según los mismos. 
 Se encarga de crear hilos y ejecutar el ftpclient.
*/
fn main () {

    /*
     Un vector de Strings para almacenar los argumentos del rastreador
    */
    let args: Vec<String> = env::args().collect();
    /*
     Un vector de punteros de String para almacenar el Prog y sus argumentos
    */
    let mut cmd_args: Vec<&str> = Vec::new();
    //static cmd:Command = Command::new("./ftclient"); //the executable name
    /*
     Opcion para imprimir. 
     0 = no imprime, 1 = imprime sin pausa, 2 = imprime pausado
    */
    let mut threads_amount = 0;

       
    if args.len() < 4 {
        error_commands_insertion();
    } 
    
    let mut index = 1;
    while index < args.len() {

        if args[index].trim() == "n" && index+1 < args.len() { //cantidad de hilos
            threads_amount = (args[index+1]).parse::<i32>().unwrap();
            index += 1;
        }
        else if args[index] == "./ftclient"{
            
        }
        else {
            cmd_args.push(args[index].trim());
            //cmd.arg(args[index].trim());
        }
        index += 1;
    }
    
    execute_threads(threads_amount, &cmd_args);
        
}

