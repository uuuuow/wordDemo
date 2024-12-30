#include "stdafx.h"
#include "MethodApi.h"

#include "Matrix.h"
// ¾ØÕó¼ÆËãº¯Êý
bool MatrixCalculation(double *pCoefficient, float *pValue, float *pUnknown, int nRows, int nCols)
{
	bool bisSucceed = false;
	CMatrix *pMatrix = new CMatrix(nRows, nCols, pCoefficient);
	if (pMatrix->InvertGaussJordan())
	{
		double *qdata = pMatrix->GetData();
		if (NULL != qdata)
		{
			CopyMemory(pCoefficient, qdata, nRows * nCols * sizeof(double));
		}
		for (int i = 0; i<nRows; i++)
		{
			pUnknown[i] = 0;
			for (int j = 0; j<nCols; j++)
			{
				pUnknown[i] += pCoefficient[i * nCols + j] * pValue[j];
			}
		}

		bisSucceed = true;
	}
	if (NULL != pMatrix)
	{
		delete pMatrix;
	}

	return bisSucceed;
}