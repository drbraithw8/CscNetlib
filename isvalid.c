// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "std.h"
#include "isvalid.h"


csc_bool_t csc_isValid_hex(const char *word)
{   char ch;
    if (word == NULL)
        return csc_FALSE;
    ch = *word;
    if (ch == '\0')
        return csc_FALSE;
    while((ch = *(word++)) != '\0')
    {   if ( !(  (ch>='0' && ch<='9')
              || (ch>='a' && ch<='f')
              || (ch>='A' && ch<='F')
              )
           )
        {   return csc_FALSE;
        }
    }
    return csc_TRUE;
}


csc_bool_t csc_isValid_int(const char *word)
{   char ch;
    if (word == NULL)
        return csc_FALSE;
    ch = *word;
    if (ch=='-')
        ch = *(++word);
    if (ch == '\0')
        return csc_FALSE;
    while((ch = *(word++)) != '\0')
        if (ch<'0' || ch>'9')
            return csc_FALSE;
    return csc_TRUE;
}


csc_bool_t csc_isValidRange_int(const char *word, int min, int max, int *value)
{   int val;
    if (!csc_isValid_int(word))
        return csc_FALSE;
    val = atoi(word);
    if (val<min || val>max)
        return csc_FALSE;
    if (value != NULL)
        *value = val;
    return csc_TRUE;
}


csc_bool_t csc_isValid_float(const char *str)
{   int has_point, has_e, has_num;
    char ch;
    if (str == NULL)
        return csc_FALSE;
    ch = *str;
    if (ch=='-' || ch=='+')
        str++;
    has_point = csc_FALSE;
    has_e = csc_FALSE;
    has_num = csc_FALSE;
    while ((ch=*(str++)) != '\0')   switch(ch)
    {   case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': 
            has_num = csc_TRUE;
            break;
        case 'e': case 'E':
            if (has_e || !has_num)
                return csc_FALSE;
            if (*str=='-' || *str=='+')
                str++;
            has_e = csc_TRUE;
            has_num = csc_FALSE;
            break;
        case '.':
            if (has_point || has_e)
                return csc_FALSE;
            has_point = csc_TRUE;
            break;
        default:
            return csc_FALSE;
    }
    return has_num;
}


csc_bool_t csc_isValidRange_float(const char *word, double min, double max, double *value)
{   double val;
    if (!csc_isValid_float(word))
        return csc_FALSE;
    val = atof(word);
    if (val<min || val>max)
        return csc_FALSE;
    if (value != NULL)
        *value = val;
    return csc_TRUE;
}


csc_bool_t csc_isValid_ipV4(const char *str)
{   struct sockaddr_in sa;
    if (str == NULL)
        return csc_FALSE;
    return inet_pton(AF_INET, str, &(sa.sin_addr)) != 0;
}

csc_bool_t csc_isValid_ipV6(const char *str)
{   struct sockaddr_in6 sa;
    if (str == NULL)
        return csc_FALSE;
    return inet_pton(AF_INET6, str, &(sa.sin6_addr)) != 0;
}


csc_bool_t csc_isValid_domain(const char *str)
{   
// Reject the empty string.
    if (str == NULL)
        return csc_FALSE;
    if (csc_streq(str,""))
        return csc_FALSE;
    int sLen=strlen(str);
 
// Look at the ends of the domain name.
    if ( str[0] == '.'
       || str[sLen-1] == '.'
       || sLen > 253
       ) 
    {   return csc_FALSE;
    }
 
// Look at each character in turn.
    int segLen = 0;
    for(int i=0; i<sLen; i++)
    {   if (str[i] == '.')
        {   if (segLen == 0)
                return csc_FALSE;
            segLen=0;
        }
        else if (  isalnum(str[i])
                || str[i]=='-' && segLen!=0 && i+1<sLen && str[i+1]!='.'
                )
        {   if (++segLen > 63)
                return csc_FALSE;
        }
        else
            return csc_FALSE; //invalid char...
    }
 
// There should be at least two segments.
    if (segLen == sLen)
        return csc_FALSE;
 
    return csc_TRUE;
}


csc_bool_t csc_isValid_decentRelPath(const char *str)
{   csc_bool_t result = csc_TRUE;
	csc_bool_t isDotsOnly = csc_TRUE;
    const char *p;
    int segLen, ch;
 
    if (str == NULL)
    {   result = csc_FALSE;
    }
    else
    { // Test each char of 'str' in turn.
        p = str;
        segLen = 0;
        
        while (csc_TRUE)
        {	ch = *p++;

            if (isalnum(ch) || ch=='_' || ch==',')
            {   segLen++;
				isDotsOnly = csc_FALSE;
            }
            else if (ch=='/')
            {	if (isDotsOnly)
				{ // Path segment consisting of zero or more dots not allowed.
                   result = csc_FALSE;
                    break;
                }
                else
                {	segLen = 0;
					isDotsOnly = csc_TRUE;
				}
            }
			else if (ch == '\0')
			{ // Path segment consisting of zero or more dots not allowed.
                if (isDotsOnly)
                { // Path segment consisting of zero or more dots not allowed.
					result = csc_FALSE;
                    break;
                }
                else
                	break;
            }
            else if (ch == '.')
            {	segLen++;
            }
            else if (ch == '-')
            { // Path segements beginning with '-' are not allowed.
                if (segLen == 0)
                {   result = csc_FALSE;
                    break;
                }
				isDotsOnly = csc_FALSE;
            	segLen++;
            }
			else if (ch == ' ')
			{ // A segment should not begin or end with a space or have consecutive.
				if (segLen==0 || *p==' ' || *p=='\0' || *p=='/')
				{   result = csc_FALSE;
					break;
				}
            	segLen++;
			}
            else
            { // General punctuation characters are not allowed.
				result = csc_FALSE;
                break;
            }
        }
 
    // Path should not be empty or end with a slash or consist only of dots.
        if (segLen == 0)
            result = csc_FALSE;
    }
 
// Its all good if we got this far.
    return result;
}


csc_bool_t csc_isValid_decentPath(const char *str)
{   csc_bool_t result;
    if (str == NULL)
        result = csc_FALSE;
    else
    {   if (*str == '/')
            str++;
        result = csc_isValid_decentRelPath(str);
    }
    return result;
}


csc_bool_t csc_isValid_decentAbsPath(const char *str)
{   csc_bool_t result;
    if (str == NULL)
        result = csc_FALSE;
    else if (*str != '/')
        result = csc_FALSE;
    else
        result = csc_isValid_decentRelPath(str+1);
    return result;
}

