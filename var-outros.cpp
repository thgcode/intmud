/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos das licen�as GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "instr.h"
#include "classe.h"
#include "objeto.h"
#include "var-outros.h"
#include "procurar.h"
#include "misc.h"

//#define DEBUG  // Mostrar e testar vetores de TVarIntTempo

TVarIntTempo ** TVarIntTempo::VetMenos = 0;
TVarIntTempo ** TVarIntTempo::VetMais = 0;
unsigned int TVarIntTempo::TempoMenos = 0;
unsigned int TVarIntTempo::TempoMais = 0;
TVarIntExec * TVarIntExec::Inicio = 0;
TVarIntExec * TVarIntExec::Fim = 0;

//------------------------------------------------------------------------------
/*
void DebugRef()
{
 for (TClasse * cl = TClasse::RBfirst(); cl; cl=TClasse::RBnext(cl))
 {
  for (TObjeto * obj = cl->ObjetoIni; obj; obj=obj->Depois)
  {
   TVarRef * vantes = 0;
   for (TVarRef * v1 = obj->VarRefIni; v1; v1=v1->Depois)
   {
    assert(v1->Pont == obj);
    assert(v1->Antes == vantes);
    vantes = v1;
   }
  }
 }
}

//------------------------------------------------------------------------------
void MostraRef(TVarRef * r)
{
    printf("  this = %p\n", r);
    if (r==0)
        return;
    printf("  Antes = %p\n", r->Antes);
    printf("  Depois = %p\n", r->Depois);
    printf("  Pont = %p\n", r->Pont);
    if (r->Pont)
        printf("  Pont->VarRefIni = %p\n", r->Pont->VarRefIni);
    if (r->Antes)
        printf("  Antes->Depois = %p\n", r->Antes->Depois);
    if (r->Depois)
        printf("  Depois->Antes = %p\n", r->Depois->Antes);
    fflush(stdout);
}
*/

//------------------------------------------------------------------------------
void TVarRef::MudarPont(TObjeto * obj)
{
// Verifica se o endere�o vai mudar
    if (obj == Pont)
        return;
    //printf("ANTES\n"); MostraRef(this); fflush(stdout);
// Retira da lista
    if (Pont)
    {
        (Antes ? Antes->Depois : Pont->VarRefIni) = Depois;
        if (Depois)
            Depois->Antes = Antes;
    }
// Coloca na lista
    if (obj)
    {
        Antes = 0;
        Depois = obj->VarRefIni;
        if (Depois)
            Depois->Antes = this;
        obj->VarRefIni = this;
    }
// Atualiza ponteiro
    Pont = obj;
    //printf("DEPOIS\n"); MostraRef(this); fflush(stdout);
}

//------------------------------------------------------------------------------
void TVarRef::Mover(TVarRef * destino)
{
    if (Pont)
    {
        (Antes ? Antes->Depois : Pont->VarRefIni) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TVarRef));
}

//------------------------------------------------------------------------------
int TVarIncDec::getInc(int numfunc)
{
    if (valor < 0)
        return (numfunc ? -valor : valor);
    unsigned int v = TempoIni + INTTEMPO_MAX*INTTEMPO_MAX - valor;
    return (v >= INTTEMPO_MAX*INTTEMPO_MAX ? INTTEMPO_MAX*INTTEMPO_MAX-1 : v);
}

//------------------------------------------------------------------------------
int TVarIncDec::getDec(int numfunc)
{
    if (valor < 0)
        return (numfunc ? -valor : valor);
    unsigned int v = valor - TempoIni;
    return (v >= INTTEMPO_MAX*INTTEMPO_MAX ? 0 : v);
}

//------------------------------------------------------------------------------
void TVarIncDec::setInc(int numfunc, int v)
{
// Acerta o sinal se for fun��o abs
    if (numfunc && (getInc(0)<0) != (v<0))
        v = -v;
// Contagem parada
    if (v < 0)
    {
        if (v <= -INTTEMPO_MAX*INTTEMPO_MAX)
            valor = -INTTEMPO_MAX*INTTEMPO_MAX+1;
        else
            valor = v;
        return;
    }
// Contagem andando
    if (v >= INTTEMPO_MAX*INTTEMPO_MAX)
        v = INTTEMPO_MAX*INTTEMPO_MAX-1;
    valor = TempoIni + INTTEMPO_MAX*INTTEMPO_MAX - v;
}

//------------------------------------------------------------------------------
void TVarIncDec::setDec(int numfunc, int v)
{
// Acerta o sinal se for fun��o abs
    if (numfunc && (getDec(0)<0) != (v<0))
        v = -v;
// Contagem parada
    if (v < 0)
    {
        if (v <= -INTTEMPO_MAX*INTTEMPO_MAX)
            valor = -INTTEMPO_MAX*INTTEMPO_MAX+1;
        else
            valor = v;
        return;
    }
// Contagem andando
    if (v >= INTTEMPO_MAX*INTTEMPO_MAX)
        v = INTTEMPO_MAX*INTTEMPO_MAX-1;
    valor = TempoIni + v;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncInc(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "abs")==0)
    {
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = 1;
        return true;
    }
    if (comparaZ(nome, "pos")==0)
    {
        int valor = getInc(0);
        if (valor<0)
            setInc(0, -valor);
        return false;
    }
    if (comparaZ(nome, "neg")==0)
    {
        int valor = getInc(0);
        if (valor>0)
            setInc(0, -valor);
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncDec(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "abs")==0)
    {
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = 1;
        return true;
    }
    if (comparaZ(nome, "pos")==0)
    {
        int valor = getDec(0);
        if (valor<0)
            setDec(0, -valor);
        return false;
    }
    if (comparaZ(nome, "neg")==0)
    {
        int valor = getDec(0);
        if (valor>0)
            setDec(0, -valor);
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncVetorInc(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int numero = 0;
    if (Instr::VarAtual > v)
    {
        numero = v[1].getInt();
        if (numero >= INTTEMPO_MAX*INTTEMPO_MAX)
            numero = INTTEMPO_MAX*INTTEMPO_MAX-1;
    }
    numero = TempoIni + INTTEMPO_MAX*INTTEMPO_MAX - numero;
    for (int x=0; x<total; x++)
        this[x].valor = numero;
    return false;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncVetorDec(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int numero = 0;
    if (Instr::VarAtual > v)
    {
        numero = v[1].getInt();
        if (numero >= INTTEMPO_MAX*INTTEMPO_MAX)
            numero = INTTEMPO_MAX*INTTEMPO_MAX-1;
    }
    numero = TempoIni + numero;
    for (int x=0; x<total; x++)
        this[x].valor = numero;
    return false;
}

//------------------------------------------------------------------------------
void TVarIntTempo::PreparaIni()
{
    if (TVarIntTempo::VetMenos)
        return;
    VetMenos = new TVarIntTempo*[INTTEMPO_MAX];
    VetMais = new TVarIntTempo*[INTTEMPO_MAX];
    memset(VetMenos, 0, sizeof(TVarIntTempo*)*INTTEMPO_MAX);
    memset(VetMais, 0, sizeof(TVarIntTempo*)*INTTEMPO_MAX);
}

//------------------------------------------------------------------------------
int TVarIntTempo::TempoEspera()
{
#ifdef DEBUG
    DebugVet(true);
#endif
    int menos = TempoMenos;
    int total = 0;
    for (; menos<INTTEMPO_MAX && total<600; menos++,total++)
        if (VetMenos[menos])
            return total;
    return total;
}

//------------------------------------------------------------------------------
void TVarIntTempo::ProcEventos(int TempoDecorrido)
{
#ifdef DEBUG
    printf("Tempo %d\n", TempoDecorrido); fflush(stdout);
#endif
    while (TempoDecorrido-- > 0)
    {
    // Avan�a TempoMenos
    // Move objetos de VetMais para VetMenos se necess�rio
        if (TempoMenos < INTTEMPO_MAX-1)
            TempoMenos++;
        else
        {
            TempoMenos = 0;
            TempoMais = (TempoMais + 1) % INTTEMPO_MAX;
            while (true)
            {
                TVarIntTempo * obj = VetMais[TempoMais];
                if (obj==0)
                    break;
            // Move objeto da lista ligada VetMais para VetMenos
                VetMais[TempoMais] = obj->Depois;
                int menos = obj->IndiceMenos;
                obj->Antes = 0;
                obj->Depois = VetMenos[menos];
                VetMenos[menos] = obj;
                if (obj->Depois)
                    obj->Depois->Antes = obj;
            }
        }
    // Inverte a ordem de VetMenos[TempoMenos]
    // Dessa forma, vari�veis alteradas primeiro geram eventos primeiro
        TVarIntTempo * obj1 = VetMenos[TempoMenos];
        if (obj1)
        {
            while (true)
            {
                TVarIntTempo * x = obj1->Depois;
                obj1->Depois = obj1->Antes;
                obj1->Antes = x;
                if (x==0) break;
                obj1 = x;
            }
            VetMenos[TempoMenos] = obj1;
        }
    // Chama eventos dos objetos em VetMenos[TempoMenos]
        while (true)
        {
            TVarIntTempo * obj = VetMenos[TempoMenos];
            if (obj==0)
                break;
        // Tira objeto da lista ligada
            VetMenos[TempoMenos] = obj->Depois;
            if (obj->Depois)
                obj->Depois->Antes = 0;
            obj->Depois = 0;
        // Gera evento
            bool prossegue = false;
            if (obj->b_objeto)
            {
                char mens[80];
                sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
                prossegue = Instr::ExecIni(obj->endobjeto, mens);
            }
            else if (obj->endclasse)
            {
                char mens[80];
                sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
                prossegue = Instr::ExecIni(obj->endclasse, mens);
            }
            if (prossegue==false)
                continue;
                // Cria argumento: �ndice
            Instr::ExecArg(obj->indice);
                // Executa
            Instr::ExecX();
            Instr::ExecFim();
        }
    }
}

//------------------------------------------------------------------------------
int TVarIntTempo::getValor(int numfunc)
{
    if (parado)
        return (numfunc ? -ValorParado : ValorParado);
    if (Antes==0 && VetMenos[IndiceMenos]!=this &&
                    VetMais[IndiceMais]!=this)
        return 0;
    int valor = ((IndiceMais - TempoMais) * INTTEMPO_MAX +
            IndiceMenos - TempoMenos);
    if (valor<0)
        valor += INTTEMPO_MAX * INTTEMPO_MAX;
    return valor;
}

//------------------------------------------------------------------------------
void TVarIntTempo::setValor(int numfunc, int valor)
{
// Tira objeto das listas ligadas
#ifdef DEBUG
    DebugVet(false);
#endif
// Acerta o sinal se for fun��o abs
    if (numfunc && (getValor(0)<0) != (valor<0))
        valor = -valor;
// Retira da lista
    if (!parado)
    {
        if (Antes)
            Antes->Depois = Depois;
        else if (VetMenos[IndiceMenos]==this)
            VetMenos[IndiceMenos] = Depois;
        else if (VetMais[IndiceMais]==this)
            VetMais[IndiceMais] = Depois;
        if (Depois)
            Depois->Antes = Antes, Depois=0;
    }
    Antes = 0;
#ifdef DEBUG
    DebugVet(false);
#endif
    parado = false;
// Valores negativos
    if (valor <= 0)
    {
        if (valor==0)
            return;
        parado = true;
        if (valor <= -INTTEMPO_MAX * INTTEMPO_MAX)
            valor = -INTTEMPO_MAX * INTTEMPO_MAX + 1;
        ValorParado = valor;
        return;
    }
// Valor m�ximo
    if (valor >= INTTEMPO_MAX * INTTEMPO_MAX)
        valor = INTTEMPO_MAX * INTTEMPO_MAX - 1;
// Acerta IndiceMenos e IndiceMais
    valor += TempoMais * INTTEMPO_MAX + TempoMenos;
    IndiceMenos = valor % INTTEMPO_MAX;
    IndiceMais = valor / INTTEMPO_MAX % INTTEMPO_MAX;
// Coloca na lista apropriada
    if (IndiceMais != TempoMais || IndiceMenos <= TempoMenos)
    {
        Depois = VetMais[IndiceMais];
        VetMais[IndiceMais] = this;
    }
    else
    {
        Depois = VetMenos[IndiceMenos];
        VetMenos[IndiceMenos] = this;
    }
    if (Depois)
        Depois->Antes = this;
#ifdef DEBUG
    DebugVet(false);
#endif
}

//------------------------------------------------------------------------------
void TVarIntTempo::Mover(TVarIntTempo * destino)
{
    if (!parado)
    {
        if (Depois)
            Depois->Antes = destino;
        if (Antes)
            Antes->Depois = destino;
        else if (VetMenos[IndiceMenos]==this)
            VetMenos[IndiceMenos] = destino;
        else if (VetMais[IndiceMais]==this)
            VetMais[IndiceMais] = destino;
#ifdef DEBUG
        DebugVet(false);
#endif
    }
    memmove(destino, this, sizeof(TVarIntTempo));
}

//------------------------------------------------------------------------------
void TVarIntTempo::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
}

//------------------------------------------------------------------------------
bool TVarIntTempo::Func(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "abs")==0)
    {
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = 1;
        return true;
    }
    if (comparaZ(nome, "pos")==0)
    {
        int valor = getValor(0);
        if (valor<0)
            setValor(0, -valor);
        return false;
    }
    if (comparaZ(nome, "neg")==0)
    {
        int valor = getValor(0);
        if (valor>0)
            setValor(0, -valor);
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarIntTempo::FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const int numero = (Instr::VarAtual <= v ? 0 : v[1].getInt());
    for (int x=0; x<total; x++)
        this[x].setValor(0, numero);
    return false;
}

//------------------------------------------------------------------------------
void TVarIntTempo::DebugVet(bool mostrar)
{
    if (mostrar)
        printf("VetMenos: ");
    for (int x=0; x<INTTEMPO_MAX; x++)
        if (VetMenos[x])
        {
            int y=0;
            TVarIntTempo * objini=0, * obj=VetMenos[x];
            for (; obj; obj=obj->Depois)
            {
                assert(obj->Antes == objini);
                objini = obj, y++;
            }
            if (mostrar)
                printf("[%d,%d]", x, y);
        }
    if (mostrar)
        printf("\nVetMais: ");
    for (int x=0; x<INTTEMPO_MAX; x++)
        if (VetMais[x])
        {
            int y=0;
            TVarIntTempo * objini=0, * obj=VetMais[x];
            for (; obj; obj=obj->Depois)
            {
                assert(obj->Antes == objini);
                objini = obj, y++;
            }
            if (mostrar)
                printf("[%d,%d]", x, y);
        }
    if (mostrar)
        { printf("\n"); fflush(stdout); }
}

//------------------------------------------------------------------------------
void TVarIntExec::ProcEventos()
{
    while (true)
    {
    // Verifica se tem objeto na lista
        TVarIntExec * obj = Inicio;
        if (obj==0)
            return;

    // Retira objeto da lista
        Inicio = obj->Depois;
        if (Inicio)
            Inicio->Antes = 0;
        else
            Fim = 0;
        obj->Depois = 0;

    // Gera evento
        if (obj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
            if (!Instr::ExecIni(obj->endobjeto, mens))
                continue;
        }
        else if (obj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
            if (!Instr::ExecIni(obj->endclasse, mens))
                continue;
        }
            // Cria argumento: �ndice
        Instr::ExecArg(obj->indice);
            // Executa
        Instr::ExecX();
        Instr::ExecFim();
    }
}

//------------------------------------------------------------------------------
int TVarIntExec::getValor()
{
    return (Antes || Inicio==this);
}

//------------------------------------------------------------------------------
void TVarIntExec::setValor(int valor)
{
    if (Antes || Inicio==this) // Se a vari�vel for 1
    {
        if (valor)
            return;
        (Antes ? Antes->Depois : Inicio) = Depois;
        (Depois ? Depois->Antes : Fim) = Antes;
        Antes = Depois = 0;
    }
    else if (valor) // Se a vari�vel for 0 e valor n�o for 0
    {
        Antes = Fim;
        Depois = 0;
        (Antes ? Antes->Depois : Inicio) = this;
        Fim = this;
    }
}

//------------------------------------------------------------------------------
void TVarIntExec::Mover(TVarIntExec * destino)
{
    if (Antes || Inicio==this)
    {
        (Antes ? Antes->Depois : Inicio) = destino;
        (Depois ? Depois->Antes : Fim) = destino;
    }
    memmove(destino, this, sizeof(TVarIntExec));
}

//------------------------------------------------------------------------------
void TVarIntExec::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
}

//------------------------------------------------------------------------------
bool FuncVetorTxt(TVariavel * v, const char * nome)
{
// Obt�m n�mero de vari�veis e tamanho de uma vari�vel
    int numvar = (unsigned char)v->defvar[Instr::endVetor];
    int tamvar = (unsigned char)v->defvar[Instr::endExtra] + 2;
    if (v->defvar[2] == Instr::cTxt2)
        tamvar += 256;
// Texto do vetor
    if (comparaZ(nome, "texto")==0)
    {
    // Obt�m �ndice inicial e a quantidade de vari�veis
        int ini = 0;
        int total = numvar;
        if (Instr::VarAtual >= v+1)
        {
            ini = v[1].getInt();
            if (ini<0)
                ini=0;
            if (Instr::VarAtual >= v+2)
            {
                total = v[2].getInt() + 1;
                if (total>numvar)
                    total=numvar;
            }
        }
        total -= ini;
    // Cria vari�vel
        const char * origem = v->end_char + ini * tamvar;
        Instr::ApagarVar(v);  // N�o tem import�ncia que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total>0; total--,origem+=tamvar)
        {
            //printf("texto = ["); fflush(stdout);
            //for (const char * x=origem; *x; x++)
            //    if (*(signed char*)x >= ' ')
            //        putchar(*x);
            //    else
            //        printf("(%02X)", *(unsigned char*)x);
            //printf("]\n"); fflush(stdout);
            for (const char * o = origem; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a vari�vel
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Divide em palavras
    if (comparaZ(nome, "palavras")==0)
    {
    // Obt�m n�mero de palavras (par�metro de entrada)
        int total = numvar;
        if (Instr::VarAtual < v+1)
            return false;
        if (Instr::VarAtual >= v+2)
        {
            total = v[2].getInt();
            if (total>numvar)
                total=numvar;
        }
    // Copia o texto obtendo o n�mero de palavras
        int palavras = 0;
        const char * origem = v[1].getTxt();
        char * destino = v->end_char;
        for (; total>1; total--)
        {
            while (*origem==' ' || *origem==Instr::ex_barra_n)
                origem++;
            if (*origem==0)
                break;
            char * d = destino;
            destino += tamvar, palavras++;
            for (; *origem && *origem!=' ' &&
                    *origem!=Instr::ex_barra_n; origem++)
                if (d<destino-1)
                    *d++ = *origem;
            *d = 0;
        }
    // Copia o �ltimo texto
        while (*origem==' ' || *origem==Instr::ex_barra_n)
            origem++;
        if (*origem)
        {
            char * d = destino;
            destino += tamvar, palavras++;
            while (*origem && d<destino-1)
                *d++ = *origem++;
            while (*origem==' ' || *origem==Instr::ex_barra_n)
                origem++;
            if (*origem==0)
            {
                for (d--; d>=destino-tamvar; d--)
                    if (*d!=' ' && *d!=Instr::ex_barra_n)
                        break;
                d++;
            }
            *d = 0;
        }
    // Limpa pr�ximas vari�veis do vetor
        for (; palavras < numvar; destino += tamvar, numvar--)
            *destino=0;
    // Retorna o n�mero de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(palavras);
    }
// Divide em linhas
    if (comparaZ(nome, "linhas")==0)
    {
    // Obt�m n�mero m�nimo de colunas (par�metro de entrada)
        int colunas = tamvar - 1;
        if (Instr::VarAtual < v+1)
            return false;
        if (Instr::VarAtual >= v+2)
        {
            colunas = v[2].getInt();
            if (colunas>tamvar-1)
                colunas=tamvar-1;
            if (colunas<0)
                colunas=0;
        }
    // Copia o texto
        const char * origem = v[1].getTxt();
        char * destino = v->end_char;
        int linha = 0;  // N�mero de linhas copiadas
        colunas -= tamvar-1; // Quantas colunas recuar do final do texto
        while (linha<numvar)
        {
            char * d = destino;
            linha++, destino+=tamvar;
        // Copia linha
            while (*origem && *origem!=Instr::ex_barra_n && d<destino)
                *d++ = *origem++;
        // Linha cabe na vari�vel
            if (d<destino)
            {
                *d=0;
            // Fim da linha
                if (*origem==Instr::ex_barra_n)
                {
                    origem++;
                    continue;
                }
            // Fim do texto
                break;
            }
        // Obt�m aonde deve dividir a linha
            d--,origem--; // Recua um caracter
            int x;
            for (x=-1; x>=colunas; x--)
                if (origem[x]==' ')
                    break;
            if (x<colunas)
                for (x=-1; x>=colunas; x--)
                    if (origem[x]=='.')
                        break;
        // Divide a linha
            if (x>=colunas)
            {
                x++;
                origem += x;
                d += x;
            }
            *d=0;
        }
    // Limpa pr�ximas vari�veis do vetor
        for (; linha < numvar; destino += tamvar, numvar--)
            *destino=0;
    // Retorna o n�mero de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(linha);
    }
// Limpa as vari�veis do vetor
    if (comparaZ(nome, "limpar")==0)
    {
        char * destino = v->end_char;
        const char * texto = "";
        if (Instr::VarAtual > v)
            texto = v[1].getTxt();
        for (; numvar>0; numvar--, destino+=tamvar)
            copiastr(destino, texto, tamvar);
        return false;
    }
// Junta texto
    if (comparaZ(nome, "juntar")==0)
    {
    // Obt�m o texto delimitador
        int total = numvar;
        char mens[4096];
        *mens=0;
        if (Instr::VarAtual >= v+1)
        {
            copiastr(mens, v[1].getTxt(), sizeof(mens));
            if (Instr::VarAtual >= v+2)
            {
                total = v[2].getInt();
                if (total>numvar)
                    total=numvar;
            }
        }
    // Cria vari�vel
        const char * origem = v->end_char;
        Instr::ApagarVar(v);  // N�o tem import�ncia que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total>0; total--,origem+=tamvar)
        {
            for (const char * o = origem; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
            if (total<=1)
                break;
            for (const char * o = mens; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a vari�vel
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Separa texto
    int valor = 10;
    if (comparaZ(nome, "separar")==0)
        valor=0;
    else if (comparaZ(nome, "separarmai")==0)
        valor=1;
    else if (comparaZ(nome, "separardif")==0)
        valor=2;
    if (valor!=10)
    {
        int indice = 0;
        int indmax = numvar;
        char * destino = v->end_char;
        while (true)
        {
            if (Instr::VarAtual < v+2)
                break;
            if (Instr::VarAtual >= v+3)
            {
                indmax = v[3].getInt();
                if (indmax<1)
                    break;
                if (indmax>numvar)
                    indmax=numvar;
            }
        // Obt�m delimitador
            TProcurar proc;
            const char * origem = v[2].getTxt();
            int tampadrao = strlen(origem);
            if (!proc.Padrao(origem, valor))
                break;
        // Obt�m texto
            origem = v[1].getTxt();
            int tam = strlen(origem);
        // Anota texto
            while (true)
            {
                indice++;
                int posic = (indice>=indmax ? -1 : proc.Proc(origem, tam));
                if (posic<0) // N�o encontrou
                {
                    copiastr(destino, origem, tamvar);
                    destino += tamvar;
                    break;
                }
                int total = (posic < tamvar-1 ? posic : tamvar-1);
                memcpy(destino, origem, total);
                destino[total] = 0;
                origem += posic + tampadrao;
                tam -= posic + tampadrao;
                destino += tamvar;
            }
            break;
        }
        // Limpa pr�ximas vari�veis do vetor
        for (; indice < numvar; destino += tamvar, numvar--)
            *destino=0;
        // Retorna o n�mero de vari�veis
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(indice);
    }
// TxtRemove
    if (comparaZ(nome, "txtremove")==0)
    {
        if (Instr::VarAtual < v+1)
            return false;
        int remove = txtRemove(v[1].getTxt()); // O que deve remover
        if (remove==0)
            return false;
        char * destino = v->end_char;
        for (; numvar>0; numvar--, destino+=tamvar)
            txtRemove(destino, destino, tamvar, remove);
        return false;
    }
    return false;
}

//const char Int1_Valor[] = { 8, 0, Instr::cInt1, 0, 0, 1, '=', '0', 0 };

//------------------------------------------------------------------------------
bool FuncVetorInt1(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")==0)
    {
        unsigned int total = (unsigned char)v->defvar[Instr::endVetor];
        unsigned char * p = (unsigned char*)v->end_char;
        unsigned char bitmask = 1 << v->numbit;
    // Preencher com 1
        if (Instr::VarAtual > v && v[1].getBool())
        {
            if (bitmask > 1)
            {
                for (; total>0 && bitmask; total--,bitmask<<=1)
                    *p |= bitmask;
                p++;
            }
            while (total>=8)
                *p++=0xFF, total-=8;
            for (bitmask=1; total>0; total--,bitmask<<=1)
                *p |= bitmask;
            return false;
        }
    // Preencher com 0
        if (bitmask > 1)
        {
            for (; total>0 && bitmask; total--,bitmask<<=1)
                *p &= ~bitmask;
            p++;
        }
        while (total>=8)
            *p++=0, total-=8;
        for (bitmask=1; total>0; total--,bitmask<<=1)
            *p &= ~bitmask;
        return false;
    }
    if (comparaZ(nome, "bits")==0)
    {
        Instr::ApagarVar(v+1);
        v->indice = 0;
        v->numfunc = 1;
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
int GetVetorInt1(TVariavel * v)
{
    unsigned int bitnum = v->numbit;
    unsigned char * p = (unsigned char *)v->end_char;
    int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (total > 32)
        total = 32;
// Primeiro byte
    if (total>0 && bitnum)
    {
        valor = *p++ >> bitnum;
        bitnum = 8-bitnum;
        switch (total)
        {
        case 1: valor &= 1; break;
        case 2: valor &= 3; break;
        case 3: valor &= 7; break;
        case 4: valor &= 15; break;
        case 5: valor &= 31; break;
        case 6: valor &= 63; break;
        case 7: valor &= 127; break;
        }
        total -= bitnum;
    }
// Demais bytes
    for (; total>0 && bitnum<32; total-=8,bitnum+=8)
    {
        switch (total)
        {
        case 1: valor |= (*p & 1) << bitnum; break;
        case 2: valor |= (*p & 3) << bitnum; break;
        case 3: valor |= (*p & 7) << bitnum; break;
        case 4: valor |= (*p & 15) << bitnum; break;
        case 5: valor |= (*p & 31) << bitnum; break;
        case 6: valor |= (*p & 63) << bitnum; break;
        case 7: valor |= (*p & 127) << bitnum; break;
        default: valor |= *p++ << bitnum;
        }
    }
    return valor;
}

//------------------------------------------------------------------------------
void SetVetorInt1(TVariavel * v, int valor)
{
    unsigned int bitnum = v->numbit;
    unsigned char * p = (unsigned char *)v->end_char;
    int total = (unsigned char)v->defvar[Instr::endVetor];
    if (total > 32)
        total = 32;
// Altera as vari�veis
    while (total > 0)
    {
        int bitmask = 255;
        switch (total)
        {
        case 1: bitmask = 1; break;
        case 2: bitmask = 3; break;
        case 3: bitmask = 7; break;
        case 4: bitmask = 15; break;
        case 5: bitmask = 31; break;
        case 6: bitmask = 63; break;
        case 7: bitmask = 127; break;
        }
    // Bits j� est�o alinhados
        if (bitnum == 0)
        {
            *p &= ~bitmask;
            *p |= (valor & bitmask);
            p++, total-=8, valor>>=8;
            continue;
        }
    // Bits n�o est�o alinhados
        *p &= ~(bitmask << bitnum);
        *p |= (valor & bitmask) << bitnum;
        p++, total-=8-bitnum, valor>>=8-bitnum;
        bitnum = 0;
    }
}

//------------------------------------------------------------------------------
bool FuncVetorInt8(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    char * ender = v->end_char;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor<-0x80)
            valor = -0x80;
        else if (valor>0x7F)
            valor = 0x7F;
    }
    memset(ender, valor, total);
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorUInt8(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    char * ender = v->end_char;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor<0)
            valor = 0;
        else if (valor>0xFF)
            valor = 0xFF;
    }
    memset(ender, valor, total);
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorInt16(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    short * ender = v->end_short;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor<-0x8000)
            valor = -0x8000;
        else if (valor>0x7FFF)
            valor = 0x7FFF;
    }
    if (valor==0)
        memset(ender, 0, 2*total);
    else
        for (int x=0; x<total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorUInt16(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    unsigned short * ender = v->end_ushort;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor<0)
            valor = 0;
        else if (valor>0xFFFF)
            valor = 0xFFFF;
    }
    if (valor==0)
        memset(ender, 0, 2*total);
    else
        for (int x=0; x<total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorInt32(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    int * ender = v->end_int;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const int valor = (Instr::VarAtual <= v ? 0 : v[1].getInt());
    if (valor==0)
        memset(ender, 0, 4*total);
    else
        for (int x=0; x<total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorUInt32(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    unsigned int * ender = v->end_uint;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    unsigned int valor = 0;
    if (Instr::VarAtual > v)
    {
        const double val_double = v[1].getDouble();
        if (val_double > 0xFFFFFFFFLL)
            valor = 0xFFFFFFFFLL;
        else if (val_double > 0)
            valor = (unsigned int)val_double;
    }
    if (valor==0)
        memset(ender, 0, 4*total);
    else
        for (int x=0; x<total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorReal(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    float * ender = v->end_float;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const float valor = (Instr::VarAtual <= v ? 0 : v[1].getDouble());
    for (int x=0; x<total; x++)
        ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorReal2(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    double * ender = v->end_double;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const double valor = (Instr::VarAtual <= v ? 0 : v[1].getDouble());
    for (int x=0; x<total; x++)
        ender[x] = valor;
    return false;
}
