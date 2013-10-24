/* 
 Toussaint Euclidean Rhythm for Max
 from "The Euclidean Algorithm Generates Traditional Musical Rhythms" by Godfried Toussaint
 and "The Theory of Rep-Rate Pattern Generation in the SNS Timing System" by E. Bjorklund
 Phillip Hermans April 2013
 */

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include <stdlib.h>

////////////////////////// object struct
typedef struct _euclid 
{
	t_object ob;			// the object itself (must be first)
	
	void *m_outlet1;
	
	t_atom *rhythm;
	
	long l_N; // number of pulses
	long l_M; // number of timing slots
	long l_rotate; // number of slots to rotate by
	
	long *l_bitmap; // bitmap of rhythm
	long *l_count;
	long *l_remainder; 
	
	bool flip; // flip the 1's and 0's if N > M/2
	
} t_euclid;

///////////////////////// function prototypes
//// standard set
void *euclid_new(t_symbol *s, long argc, t_atom *argv);
void euclid_free(t_euclid *x);
void euclid_assist(t_euclid *x, void *b, long m, long a, char *s);

// simple shit
void simpleBang(t_euclid *s);
void setM(t_euclid *s, t_symbol *sym, long argc, t_atom *argv);
void setN(t_euclid *s, t_symbol *sym, long argc, t_atom *argv);
void setRotate(t_euclid *s, t_symbol *sym, long argc, t_atom *argv);

// recursive euclid algorithm
long recEuclid(long m, long k);
void compute_bitmap(t_euclid *s);
void build_string(t_euclid *s, int level);

//////////////////////// global class pointer variable
void *euclid_class;


int main(void)
{	
	// object initialization, OLD STYLE
	// setup((t_messlist **)&euclid_class, (method)euclid_new, (method)euclid_free, (short)sizeof(t_euclid), 
	//		0L, A_GIMME, 0);
    // addmess((method)euclid_assist,			"assist",		A_CANT, 0);  
	
	// object initialization, NEW STYLE
	t_class *c;
	
	c = class_new("euclid", (method)euclid_new, (method)euclid_free, (long)sizeof(t_euclid), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)euclid_assist, "assist", A_CANT, 0);  
	class_addmethod(c, (method)simpleBang, "bang", 0);
	class_addmethod(c, (method)setN, "pulses", A_GIMME, 0);
	class_addmethod(c, (method)setM, "steps", A_GIMME, 0);
	class_addmethod(c, (method)setRotate, "rotate", A_GIMME, 0);
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	euclid_class = c;

	post("I am the Toussaint-Euclid rhythm generator object");
	return 0;
}

void euclid_assist(t_euclid *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "I am inlet %ld", a);
	} 
	else {	// outlet
		sprintf(s, "I am outlet %ld", a); 			
	}
}

void euclid_free(t_euclid *x)
{
	free(x->l_bitmap);
	free(x->l_count);
	free(x->l_remainder);
}

void simpleBang(t_euclid *s) // compute the g'damn Euclidean rhythm
{
	if( s->l_M <= 0)
	{ return; }
	// first allocate memory for the g'damn string
	s->l_bitmap = realloc(s->l_bitmap, s->l_M*sizeof(long));
	s->l_count = realloc(s->l_count, s->l_M*sizeof(long));
	s->l_remainder = realloc(s->l_remainder, s->l_M*sizeof(long));
	s->rhythm = realloc(s->rhythm, s->l_M*sizeof(t_atom));
	int i;
	for(i = 0; i < s->l_M; i++) // fill array with bla
	{ 
		s->l_bitmap[i] = 2;
		s->l_count[i] = 0;
		s->l_remainder[i] = 0;
		atom_setlong(s->rhythm+i,0);
	}
	
	if( s->l_N <= 0 ) // return empty array
	{ return; }
	
	compute_bitmap(s);
	if (s->l_rotate < 0 )
	{ s->l_rotate = s->l_M + (s->l_rotate % s->l_M); }
	int start = findDude(s,1); // rotate so it starts with onset
	for(i = start; i < s->l_M; i++)
	{ 
		atom_setlong(s->rhythm+(i-start), s->l_bitmap[(i+s->l_rotate)%s->l_M]); 
	}
	for(i = 0; i < start; i++)
	{
		atom_setlong(s->rhythm+(i+s->l_M-start), s->l_bitmap[(i+s->l_rotate)%s->l_M]);
	}
	
	outlet_list(s->m_outlet1, NULL, s->l_M, s->rhythm);
	
	
}

void compute_bitmap(t_euclid *s) // slots and pulses
{
	long divisor = s->l_M - s->l_N;
	s->l_remainder[0] = s->l_N; 
	int level = 0;
	do {
		s->l_count[level] = divisor / s->l_remainder[level];
		s->l_remainder[level + 1] = divisor % s->l_remainder[level];
		divisor = s->l_remainder[level];
		level++;
	} while ( s->l_remainder[level] > 1 );
	
	s->l_count[level] = divisor;
	
	int step = 0;
	build_string(s,level);
		
}

void build_string(t_euclid *s, int level)
{
	int step = findDude(s,2);
	if (level == -1)
	{ 
		s->l_bitmap[step] = 0;
	}
	else if (level == -2)
	{
		s->l_bitmap[step] = 1;
	}
	else 
	{
		int i;
		for (i = 0; i < s->l_count[level]; i++)
		{
			build_string(s, level-1);
		}
		if (s->l_remainder[level] != 0)
		{
			build_string(s, level-2);
		}
	}

	
}

int findDude(t_euclid *s, int val)
{
	int i;
	for(i = 0; i < s->l_M; i++)
	{
		if ( s->l_bitmap[i] == val)
		{ return i; }
	}
}

void setM(t_euclid *s, t_symbol *sym, long argc, t_atom *argv) // set number of timing slots
{
	s->l_M = atom_getlong(argv);
	post("Number of timing slots set to: %lu", s->l_M);
}

void setN(t_euclid *s, t_symbol *sym, long argc, t_atom *argv) // set number of pulses
{
	s->l_N = atom_getlong(argv);
	post("Number of pulses set to: %lu", s->l_N);
}

void setRotate(t_euclid *s, t_symbol *sym, long argc, t_atom *argv) // set number of pulses
{
	s->l_rotate = atom_getlong(argv);
	post("Rotate by: %lu", s->l_rotate);
}

void *euclid_new(t_symbol *s, long argc, t_atom *argv)
{
	t_euclid *x = NULL;
    long i;

	// object instantiation, NEW STYLE
	if (x = (t_euclid *)object_alloc(euclid_class)) {
        object_post((t_object *)x, "a new %s object was instantiated: 0x%X", s->s_name, x);
        object_post((t_object *)x, "it has %ld arguments", argc);
		x = (t_euclid *)object_alloc(euclid_class);
		
		x->m_outlet1 = listout((t_euclid *)x);
        
        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG) {
                object_post((t_object *)x, "arg %ld: long (%ld)", i, atom_getlong(argv+i));
            } else if ((argv + i)->a_type == A_FLOAT) {
                object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
            } else if ((argv + i)->a_type == A_SYM) {
                object_post((t_object *)x, "arg %ld: symbol (%s)", i, atom_getsym(argv+i)->s_name);
            } else {
                object_error((t_object *)x, "forbidden argument");
            }
        }
	}
	// default values
	x->l_M = 12;
	x->l_N = 5;
	x->l_rotate = 0;
	return (x);
}
