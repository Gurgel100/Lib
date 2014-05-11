/*
 * stdlib.c
 *
 *  Created on: 28.07.2012
 *      Author: pascal
 */

#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#define HEAP_RESERVED	0x01
#define HEAP_FLAGS		0xAA

typedef struct{
		void *Prev;
		void *Next;
		size_t Length;
		uint8_t Flags;		//Bit 0: Reserviert (1) oder Frei (0)
							//(Flags): Alle ungeraden Bits 1 und alle geraden 0
}heap_t;

/*static*/ uint64_t Heap_Entries = 0;	//wegen Hack ist 'static' nicht da
static void *Heap_Base;

inline void *AllocPage(size_t Pages);
inline void FreePage(void *Address, size_t Pages);
void setupNewHeapEntry(heap_t *old, heap_t *new);

void *calloc(size_t nitems, size_t size)
{
	void *Address = malloc(nitems * size);
	if(Address == NULL) return NULL;
	memset(Address, 0, nitems * size);
	return Address;
}

//TODO
void free(void *ptr)
{
	heap_t *Heap, *tmpHeap;
	Heap = ptr - sizeof(heap_t);
	if(Heap->Flags == (HEAP_FLAGS | HEAP_RESERVED))
	{
		if(Heap->Prev != NULL)
		{
			tmpHeap = Heap->Prev;
			if(!(tmpHeap->Flags & HEAP_RESERVED))	//Wenn der Speicherbereich vornedran
			{									//frei ist, dann mit diesem fusionieren
				tmpHeap->Next = Heap->Next;
				tmpHeap->Length += Heap->Length + sizeof(heap_t);
				Heap = tmpHeap;
				Heap_Entries--;
			}
		}
		if(Heap->Next != NULL)
		{
			tmpHeap = Heap->Next;
			if(!(tmpHeap->Flags & HEAP_RESERVED))	//Wenn nächster Speicherbereich
			{						//hintendran frei ist, dann mit diesem fusionieren
				Heap->Next = tmpHeap->Next;
				Heap->Length += tmpHeap->Length + sizeof(heap_t);
				Heap_Entries--;
			}
		}
		Heap->Flags = HEAP_FLAGS;
		if(Heap_Entries == 1)
		{
			size_t Pages = (Heap->Length % 4096 != 0) ? Heap->Length / 4096 + 1 :
					Heap->Length /4096;
			FreePage(Heap, Pages);
		}
	}
}

/* TODO: Es kann maximal 4kb Speicher angefordert werden.
 * Es werden keine neuen Pages reserviert
 */
void *malloc(size_t size)
{
	heap_t *Heap, *tmp_Heap;
	void *Address;
	if(Heap_Entries == 0)	//Es wurde bisher kein Speicher angefordert
	{
		uint64_t Pages = (size + sizeof(heap_t)) / 4096;
		if((size + sizeof(heap_t)) % 4096 != 0) Pages++;
		Heap_Base = AllocPage(Pages);					//Eine Page reservieren
		if(Heap_Base == NULL) return NULL;
		Heap = (heap_t*)Heap_Base;
		Heap->Next = NULL;
		Heap->Prev = NULL;
		Heap->Length = Pages * 4096 - sizeof(heap_t);	//Speicher für den Header abziehen
		//Jetzt angefordeten Speicher reservieren
		Address = (void*)((uintptr_t)Heap + sizeof(heap_t));
		Heap->Flags = HEAP_FLAGS | HEAP_RESERVED;		//Reserved-Bit setzen
		tmp_Heap = Address + size;
		setupNewHeapEntry(Heap, tmp_Heap);
		tmp_Heap->Length = Heap->Length - size - sizeof(heap_t);//Heapheader abziehen
		Heap->Length = size;			//Länge des ersten Speicherbereichs aktualisieren
		Heap_Entries = 2;
	}
	else						//Es wurde schon Speicher reserviert
	{
		Heap = Heap_Base;
		register uint64_t i;
		for(i = 0; i < Heap_Entries; i++)
		{
			if((Heap->Flags & HEAP_RESERVED) || (Heap->Length < size))
			{
				Heap = Heap->Next;
				continue;		//Wenn dieser Speicherbereich reserviert ist
			}					//zum nächsten gehen
			if (Heap->Length >= size + sizeof(heap_t))
			{
				Address = (void*)((uintptr_t)Heap + sizeof(heap_t));
				tmp_Heap = Address + size;
				setupNewHeapEntry(Heap, tmp_Heap);
				tmp_Heap->Length = Heap->Length -size - sizeof(heap_t);
				Heap->Length = size;				//Länge anpassen
				Heap->Flags |= HEAP_RESERVED;		//Als reserviert markieren
				return Address;
			}
			else
			{
				Heap->Flags |= HEAP_RESERVED;	//Als reserviert markieren
				return Heap + sizeof(heap_t);
			}
			/*if(Heap->Length == size)
			{
				Heap->Flags |= HEAP_RESERVED;	//Als reserviert markieren
				Address = Heap + sizeof(heap_t);
				break;
			}
			else if(Heap->Length > size)
			{
				Address = (void*)((uintptr_t)Heap + sizeof(heap_t));
				tmp_Heap = Address + size;
				setupNewHeapEntry(Heap, tmp_Heap);
				tmp_Heap->Length = Heap->Length -size - sizeof(heap_t);
				Heap->Length = size;		//Länge anpassen
				Heap->Flags |= HEAP_RESERVED;		//Als reserviert markieren
				break;
			}
			Heap = Heap->Next; */
		}
		if (Heap->Flags & HEAP_RESERVED)
		{
			uint64_t Pages = (size + sizeof(heap_t)) / 4096;
			tmp_Heap = Heap;
			if((size + sizeof(heap_t)) % 4096 != 0) Pages++;
			Heap = AllocPage(Pages);						//Eine Page reservieren
			if(Heap == NULL) return NULL;
			Heap_Entries++;
			tmp_Heap->Next = Heap;
			Heap->Next = NULL;
			Heap->Prev = tmp_Heap;
			Heap->Length = Pages * 4096 - sizeof(heap_t);	//Speicher für den Header abziehen
			Address = (void*)((uintptr_t)Heap + sizeof(heap_t));
			Heap->Flags = HEAP_FLAGS | HEAP_RESERVED;		//Reserved-Bit setzen

			if (Heap->Length <= Pages * 4096 - 2 * sizeof(heap_t))
			{
				tmp_Heap = Address + size;
				setupNewHeapEntry(Heap, tmp_Heap);
			}
		}
		else
		{
			size_t additionalSize = size - Heap->Length;
			uint64_t Pages = additionalSize / 4096;
			if(additionalSize % 4096 != 0) Pages++;
			if(AllocPage(Pages) == NULL) return NULL;       //Eine Page reservieren
			Heap->Length += Pages * 4096;
			Address = (void*)((uintptr_t)Heap + sizeof(heap_t));
			Heap->Flags = HEAP_FLAGS | HEAP_RESERVED;		//Reserved-Bit setzen

			if (additionalSize <= Pages * 4096 - sizeof(heap_t))
			{
				tmp_Heap = Address + size;
				setupNewHeapEntry(Heap, tmp_Heap);
			}
		}
		//Neuer Speicher anfordern
		/*Heap = Heap_Base;
		register uint64_t i;
		for(i = 0; i < Heap_Entries; i++)
			Heap = Heap->Next;
		size_t AllocSize = size - Heap->Length;
		uint64_t Pages = (size % 4096 != 0) ? size / 4096 + 1 : size / 4096;
		tmp_Heap = AllocPage(Pages);*/
	}
	return Address;
}

//TODO
void *realloc(void *ptr, size_t size)
{
	heap_t *Heap, *tmpHeap;
	void *Address = NULL;
	Heap = ptr - sizeof(heap_t);
	if(Heap->Length > size)
	{
		Heap->Length = size;
		tmpHeap = Heap->Next;
		if(tmpHeap->Flags & HEAP_RESERVED)	//Wenn nächster Speicherbereich belegt ist,
		{	//dann muss dazwischen ein neuer Eintrag hinein
			tmpHeap = Heap->Next - size;
			setupNewHeapEntry(Heap, tmpHeap);
			tmpHeap->Length = Heap->Length - size;
		}
		else
		{
			tmpHeap->Length += Heap->Length - size;
		}
		Address = ptr;
	}
	else if(Heap->Length < size)
	{
	}
	return Address;
}

int abs(int x)
{
	return (x < 0) ? -x : x;
}

//Hilfsfunktionen
inline void *AllocPage(size_t Pages)
{
	void *Address;
	asm("int $0x30" :"=a"(Address) :"a"(0), "b"(Pages));
	return Address;
}

inline void FreePage(void *Address, size_t Pages)
{
	asm("int $0x30" : :"a"(1), "b"(Address), "c"(Pages));
}

void setupNewHeapEntry(heap_t *old, heap_t *new)
{
	new->Prev = old;
	if(old->Next == NULL)
		new->Next = NULL;
	else
		new->Next = old->Next;
	old->Next = new;
	new->Flags = HEAP_FLAGS;
	Heap_Entries++;
}
