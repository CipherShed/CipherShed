#include <stddef.h>
#include <stdio.h>


struct a
{
    struct b
    {
        int i;
        float j;
    }x;
    struct c
    {
        int k;  
        float l;
    }y;
}z;

int main(void)
{
	size_t offs[]={
offsetof(struct a, x.i),
offsetof(struct a, x.j),
offsetof(struct a, y.k),
offsetof(struct a, y.l),
};
for (int i=0; i<sizeof(offs)/sizeof(offs[0]); ++i) printf ("%d\t:%zu\n",i,offs[i]);

  //  printf("k = %zu %zu; i = %zu %zu\n", off_k_y, off_k_z, off_i_x, off_i_z);
    return 0;
}
