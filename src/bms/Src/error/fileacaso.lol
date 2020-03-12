 

1) the main program pulls data 
2) the program saves the data in the relative primitive values find in data.c using the acquisition functions.
    2.1) the relative values are known because each function (one that pulls data) knows which primitives to use
        for example file:ltc6813.c -> uint8_t ltc6813_read_voltages(SPI_HandleTypeDef *spi, uint16_t volts[]) {
    2.2) the acquisition function checks the new data for not nominal values, if it finds problems,
    it calls error_set passing the type of error and its offset relative to the beginning of the value's
    data structure.
		2.1) error_set checks if an error with the same type and offset already exists;
				 if false, it creates a new er_node and it inserts it at the beginning of the error list
				 and then updates the error_reference array.
				 
    ...
    when all data capture, error settings etc... are done
    a job starts looking the linked list of errors and we want to update them regarding the number of times 
    they happened and the type of the error being.
    so if an error is bad and you found it in the linked list, what are the procedures from now on?
		If the error has exceeded the maximum time, the bms should go into a HALT state and do the STACCAH STACCAH.
    So when you read the list you do not need other data that is outside the list, right? Correct
    so the thing is that the list in the whole program is accessed only in two ways:
    --the job that creates the entries, and the direction of access is form data_values to list
    --the job that checks the list, and the  direction is from list to main program to maybe halt it

    right? yup
    ok, so lets try definig the real problem

    all methods that retrive data are specific for each peripheral
    but all boils down to a function that is not specific but general, the function I'm talking about 
    is error_set/unset.
   
    ! all functions that implement some kind of data storage call those functions at the end 
    so this function shall not depend on the function that calls them.
    so we have to make the signature of the function and by that I mean what this functions (set/unset) need
    to know and to manage
    
    ABSTRACTION TIME uwuwuwuwuwuwuwuwu

    we know that
    some_specific_data_pulling_function(){
        some_specific_data_validator_function(){

            error_set/unset <- what I need to know #

        }

    }

    #
    -- type of error (probably an enum)
    -- reference to the memory cell where a possible error could be 
    (not a ref, i need an integer in order to use it in arrays)
    don't think about the index you do not need it, let me explain

    if the error_set must check if there where previous error it shall knwo where to find them
    this task on knwowing where to watch is given to the specific function, the specific functions
    knows about which data we are talkning about and where the complementary error_array_reference to a data type is 

    so for example if I'm calling 
    ltc6813_read_voltages(data_voltages){
        ltc6813_check_voltages(){
            if(voltages are over the rating){
                error_set(ERROR_TYPE,where_i_can_look_if_there_where_errors => error_voltages[cell_we_are_talking]){
                   error set checks if error_voltages[cell_we_are_talking] is void
                   if(yes) {
                       add_node of type error_status_t to list
                   }else{
                       do updated if needed
                       take the reference contained in error_voltages[cell_we_are...]
                       dereference (now we are in the list) and change the parameters of 
                       (error_voltages[cell_we_are_talking])->{
                           error_t type; /*!< Defines the type of error */
                            uint8_t offset;
                            bool active;		 /*!< True if the error is currently happening */
                            bool fatal;			 /*!< True if the error is fatal */
                            uint16_t count;		 /*!< How many times the error has occurred */
                            uint32_t time_stamp; /*!< Last time the error activated */
                       }
                   }
                }
            } else {
                call error_unset(error_voltages[cell_we_are_talking]){
                    if(error_voltages[cell_we_are_talking]==NULL) do nothing
                    else{
                        list_remove(list,error_voltages[cell_we_are_talking]); 
                    }
                }
            }
        }
    }

ltc6813_read_voltages(data_voltages){
        ltc6813_check_voltages(){
            if(voltages are over the rating){
                error_set(ERROR_TYPE,offset){
                   error set checks if error_reference[type][offset] is void
                   if(yes) {
                       add_node of type error_status_t to list
                   }else{
                       do updated if needed<
                       take the reference contained in error_voltages[cell_we_are...]
                       dereference (now we are in the list) and change the parameters of 
                       (error_voltages[type][offset])->{
                           error_t type; /*!< Defines the type of error */
                            uint8_t offset;
                            bool active;		 /*!< True if the error is currently happening */
                            bool fatal;			 /*!< True if the error is fatal */
                            uint16_t count;		 /*!< How many times the error has occurred */
                            uint32_t time_stamp; /*!< Last time the error activated */
                       }
                   }
                }
            } else {
                call error_unset(type,offset){
                    if(error_reference[type][offset]==NULL) do nothing
                    else{
                        list_remove(list,error_reference[type][offset]); 
                    }
                }
            }
        }
    }


node_t* f_returns_reference_to_node_t(node_t ** error_reference_local, index){
    uint32_t number_of_elements=0;
    //ora sistemo i puntatori
    number_of_elements = sizeof((error_reference))/sizeof(*error_reference);
    return index>number_of_elements ? *error_reference, error_reference[index]

}

	 Secondo me quello che serve a set è type, offset e timestamp
	 (anche se timestamp potrebbe prenderselo, ma se volessimo associare un timestamp al tempo di acquisizione del dato potremmo farlo)
	 unset ha bisogno di type e offset
	trallaz
//so the proble is when i set the data and i want to put it in the linked list, i don't now ho to retrive it 
when i read it? I want to see if the error exists without searching the list. Using the array this is done in O(1)
//this is the job of the erro_voltages array list
//when you are setting the error in error_set, since you know the type of error you can check the array if it has pointers.
//so I think the problem is the mapping from the type of error to the array right? Fucking yeah
//let me think
//so nice one btw .
// Loading .....................................

dc
allora
quando setti l'errore e usi la struct (facciamo finta che sia voltages), la struct contiente
-data_voltages: sono i dati
-error_voltages: sono i riferimenti alle entries dentro la linked list
quando sei nella funzione error_set, ti stai portando dentro la struct relativa al tipo di dato che usi

quindi questa struttura deve essere nota a chi chiama error_set
quando chiami error set tu sai che elemento stai gestendo o sbaglio? Sì
allora dammi ancora qualche minuto che prendo foglio e penna. k
