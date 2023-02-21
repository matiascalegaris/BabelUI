#include "Encoding.h"

//Rem Encripta una cadena de caracteres.
//Rem S = Cadena a encriptar
//Rem P = Password
//Function EncryptStr(ByVal s As String, ByVal P As String) As String
//On Error GoTo EncryptStr_Err
//Dim i  As Integer, r As String
//Dim c1 As Integer, C2 As Integer
//r = ""
//If Len(P) > 0 Then
    //For i = 1 To Len(s)
    //c1 = Asc(mid(s, i, 1))
    //If i > Len(P) Then
    //C2 = Asc(mid(P, i Mod Len(P) + 1, 1))
    //Else
    //C2 = Asc(mid(P, i, 1))
    //End If
    //c1 = c1 + C2 + 64
    //If c1 > 255 Then c1 = c1 - 256
    //r = r + Chr(c1)
//Next i
//Else
    //r = s
//End If
//EncryptStr = r
//Exit Function




std::string Encode(const std::string source, const std::string password)
{
    if (password.empty())
    {
        return source;
    }
    std::string ret;
    for (int i = 0; i < source.length(); i++)
    {
        uint8_t value = source[i];
        uint8_t enc = password[i % password.length()];
        value += enc + 64;
        ret += value;
    }
    return ret;
}

std::string Decode(const std::string source, const std::string password)
{
    std::string ret;

    if (password.empty())
    {
        return source;
    }
    for (int i = 0; i < source.length(); i++)
    {
        uint8_t value = source[i];
        uint8_t enc = password[i % password.length()];
        value = value - enc - 64;
        ret += value;
    }
    return ret;
}
