unsigned int __VERIFIER_nondet_uint( void );

int main( void )
{
    unsigned int x = __VERIFIER_nondet_uint();
    unsigned int y = __VERIFIER_nondet_uint();
    unsigned int z = __VERIFIER_nondet_uint();
    if ( x > y )
        return ((x & y) << 3) ^ z;
    return ((x | y) >> 2) ^ z;
}
