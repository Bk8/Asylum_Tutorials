
#ifndef _ORDEREDARRAY_H_
#define _ORDEREDARRAY_H_

#include <ostream>

/**
 * \brief Rendezett t�mb int-ekhez
 *
 * A rendezett t�mb el�nye, hogy logaritmikus id�ben lehet
 * keresni benne. A t�bbi m�veletre hasonl� igaz, de m�g hozz�j�n
 * a m�sol�sokb�l ad�d� plusz id�.
 */
class orderedarray
{
private:
	int* data;		/*!< Az adat */
	size_t mycap;	/*!< Mennyit tud t�rolni */
	size_t mysize;	/*!< H�ny elem van */

	//! Megmondja, hogy hova k�ne berakni
	size_t _find(int value) const;

public:
	static const size_t npos = 0xffffffff;

	orderedarray();
	orderedarray(const orderedarray& other);
	~orderedarray();

	//! Berak egy elemet, ha m�g nincs bent
	bool insert(int value);

	//! T�r�l egy elemet, ha bent van
	void erase(int value);

	//! El�re lefoglal memori�t
	void reserve(size_t newcap);

	//! Ki�riti a t�mb�t �s deallok�lja a mem�ri�t
	void destroy();

	//! Ki�riti a t�mb�t
	void clear();

	//! Ha benne van akkor az index�t adja, ha nem akkor npos-t
	size_t find(int value) const;

	//! �rt�kad�s oper�tor
	orderedarray& operator =(const orderedarray& other);

	//! Csak olvasni engedj�k
	inline const int& operator [](size_t index) const {
		return data[index];
	}

	//! Elemek sz�ma
	inline size_t size() const {
		return mysize;
	}

	//! Ki�rat� oper�tor
	friend std::ostream& operator <<(std::ostream& os, orderedarray& oa);
};

#endif
