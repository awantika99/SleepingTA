#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> // used for creating and joining threads
#include <semaphore.h> // used for notifying TA that students are present.
#include <ctype.h> // used for isdigit()

void* Students( void* student_id );
void* TA();

#define Waiting_Chairs 3
#define DEFAULT_STUDENTS 4

sem_t semphore_students;
sem_t semphore_ta;
pthread_mutex_t thread;

int chairs[3];
int number_students_waiting = 0;
int seating_nextposition = 0;
int nextposition_teaching = 0;
int sleeping_FLAG = 0; // 0 means not sleeping, 1 means sleeping


int isNumber(char number[])  // converting into numbers from characters
{
    int i;
		for ( i = 0 ; number[i] != 0; i++ )
    {
        if (!isdigit(number[i]))
            return 0;
    }
    return 1;
}

int isWaiting( int student_id ) {   // Checks whether students are waiting or not.
	int i;
	for ( i = 0; i < 3; i++ ) {
		if (chairs[i] == student_id ) { return 1; }
	}
	return 0;
}



void* TA() {    // This function explains the working of the TA

	printf( "Checking the availability of students.\n" );

	while( number_students_waiting >= 0 ) {

		//if students are waiting
		if ( number_students_waiting > 0 ) {

			sleeping_FLAG = 0;   // TA not sleeping as students are waiting
			sem_wait( &semphore_students );
			pthread_mutex_lock( &thread );

			int solving_time = rand() % 4;

			//TA helping Students.
			printf( "Students getting help for %d seconds. Waiting Time = %d.\n", solving_time, (number_students_waiting - 1) );
			printf( "Students %d getting help.\n",chairs[nextposition_teaching] );

			chairs[nextposition_teaching]=0;
			number_students_waiting--;    // decrementing the count of the students whose doubts have been resolved.
			nextposition_teaching = ( nextposition_teaching + 1 ) % Waiting_Chairs;

			sleep ( solving_time );

			pthread_mutex_unlock( &thread );
			sem_post( &semphore_ta );

		}
		
		else { // Students are not waiting. Hence, TA will start napping.

			if ( sleeping_FLAG == 0 ) {  

				printf( " TA sleeping as students are not waiting.\n" );
				sleeping_FLAG = 1;  // condition to identify TA is sleeping.

			}

		}

	}

}

void* Students( void* student_id ) {

	int id_student = *(int*)student_id;

	while( id_student ) {

		if ( isWaiting( id_student ) == 1 )    continue; //if Students is waiting, continue waiting

		//Students are programming.
		int time = rand() % 4;
		printf( "\tStudent %d is programming for %d seconds.\n", id_student, time );
		sleep( time );

		pthread_mutex_lock( &thread );

		if( number_students_waiting < Waiting_Chairs ) { // this piece of code denotes availability of vacant chairs.

			chairs[seating_nextposition] = id_student;
			number_students_waiting++;

			//Students takes a seat in the hallway.
			printf( "\t\tStudent %d takes a seat. Students waiting = %d.\n", id_student, number_students_waiting );
			seating_nextposition = (seating_nextposition + 1 ) % Waiting_Chairs;

			pthread_mutex_unlock( &thread );

			//Awake TA if he is dozing
			sem_post( &semphore_students );
			sem_wait( &semphore_ta );

		}
		else {

			pthread_mutex_unlock( &thread );

			printf( "\t\t\tStudent %d will try later.\n",id_student ); //No chairs vacant.

		}

	}

}

int main( int num1, char **num2 ){

	int i;
	int student_num;

	if (num1 > 1 ) {
		if ( isNumber( num2[1] ) == 1) {
			student_num = atoi( num2[1] );
		}
		else {
			printf("Invalid input.");
			return 0;
		}
	}
	else {
		student_num = DEFAULT_STUDENTS;
	}

	int student_ids[student_num];
	pthread_t students[student_num];
	pthread_t thread_ta;

	sem_init( &semphore_students, 0, 0 );
	sem_init( &semphore_ta, 0, 1 );

	//Creating threads.
	pthread_mutex_init( &thread, NULL );
	pthread_create( &thread_ta, NULL, TA, NULL );
	for( i = 0; i < student_num; i++ )
	{
		student_ids[i] = i + 1;
		pthread_create( &students[i], NULL, Students, (void*) &student_ids[i] );
	}

	//Joining threads
	pthread_join(thread_ta, NULL);
	for( i =0; i < student_num; i++ )
	{
		pthread_join( students[i],NULL );
	}

	return 0;
}

