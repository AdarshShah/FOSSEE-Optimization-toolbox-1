// Copyright (C) 2015 - IIT Bombay - FOSSEE
//
// This file must be used under the terms of the CeCILL.
// This source file is licensed as described in the file COPYING, which
// you should have received as part of this distribution.  The terms
// are also available at
// http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt
// Author: R.Vidyadhar & Vignesh Kannan
// Organization: FOSSEE, IIT Bombay
// Email: toolbox@scilab.in

#include "minbndNLP.hpp"
#include "sci_iofunc.hpp"

extern "C"
{
#include <api_scilab.h>
#include <Scierror.h>
#include <BOOL.h>
#include <localization.h>
#include <sciprint.h>
#include <string.h>
#include <assert.h>

using namespace std;
using namespace Ipopt;

minbndNLP::~minbndNLP()
{
	if(finalX_) delete[] finalX_;
	if(finalZl_) delete[] finalZl_;
	if(finalZu_) delete[] finalZu_;
}

//get NLP info such as number of variables,constraints,no.of elements in jacobian and hessian to allocate memory
bool minbndNLP::get_nlp_info(Index& n, Index& m, Index& nnz_jac_g, Index& nnz_h_lag, IndexStyleEnum& index_style)
{
	n=numVars_; // Number of variables
	m=numConstr_; // Number of constraints
	nnz_jac_g = 0; // No. of elements in Jacobian of constraints 
	nnz_h_lag = n*(n+1)/2; // No. of elements in lower traingle of Hessian of the Lagrangian.
	index_style=C_STYLE; // Index style of matrices

	return true;
}

//get variable and constraint bound info
bool minbndNLP::get_bounds_info(Index n, Number* x_l, Number* x_u, Index m, Number* g_l, Number* g_u)
{	
	for(Index i=0;i<n;i++)
	{
		x_l[i]=varLB_[i]+0.0000001;
		x_u[i]=varUB_[i]-0.0000001;
	}
	
        g_l=NULL;
        g_u=NULL;

	return true;
}

// return the value of the constraints: g(x)
bool minbndNLP::eval_g(Index n, const Number* x, bool new_x, Index m, Number* g)
{
  	g=NULL;
  	return true;
}

// return the structure or values of the jacobian
bool minbndNLP::eval_jac_g(Index n, const Number* x, bool new_x,Index m, Index nele_jac, Index* iRow, Index *jCol,Number* values)
{
 	if (values == NULL) 
 	{
    		// return the structure of the jacobian of the constraints
    		iRow=NULL; 
    		jCol=NULL;
  	}
  	else 
	{
		values=NULL;
  	}

  	return true;
}

//get value of objective function at vector x
bool minbndNLP::eval_f(Index n, const Number* x, bool new_x, Number& obj_value)
{
  	int* funptr=NULL; 
  	double check;
  	Index i; 
  	if(getFunctionFromScilab(1,&funptr))
  	{
		return 1;
 	}
  	char name[20]="f";
  	double obj=0;
  	const Number *xNew=x;
  	createMatrixOfDouble(pvApiCtx, 3, 1, numVars_, xNew);
  	int positionFirstElementOnStackForScilabFunction = 3;
  	int numberOfRhsOnScilabFunction = 1;
  	int numberOfLhsOnScilabFunction = 2;
  	int pointerOnScilabFunction     = *funptr;
  
  	C2F(scistring)(&positionFirstElementOnStackForScilabFunction,name,
                                                               &numberOfLhsOnScilabFunction,
                                                               &numberOfRhsOnScilabFunction,(unsigned long)strlen(name));
    if(getDoubleFromScilab(4,&check))
  	{
		return true;
	}
	if (check==1)
	{
		
		return true;
	}	
	else
	{                           
  		if(getDoubleFromScilab(3,&obj))
  		{
			sciprint("No obj value");
			return 1;
  		}
  		obj_value=obj;  
		return true;
	}
}

//get value of gradient of objective function at vector x.
bool minbndNLP::eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f)
{
  	int* gradhessptr=NULL;
  	if(getFunctionFromScilab(2,&gradhessptr))
  	{
		return 1;
  	}  
  	const Number *xNew=x;
  	Index i;
  	double t=1;
  	createMatrixOfDouble(pvApiCtx, 3, 1, numVars_, xNew);
  	createScalarDouble(pvApiCtx, 4,t);
  	int positionFirstElementOnStackForScilabFunction = 3;
  	int numberOfRhsOnScilabFunction = 2;
  	int numberOfLhsOnScilabFunction = 2;
  	int pointerOnScilabFunction     = *gradhessptr;
	char name[20]="gradhess";

  	C2F(scistring)(&positionFirstElementOnStackForScilabFunction,name,
                                                               &numberOfLhsOnScilabFunction,
                                                               &numberOfRhsOnScilabFunction,(unsigned long)strlen(name));
 	
                               
  	double* resg;
  	double check; 
  	int x0_rows,x0_cols;                           
  	
	if(getDoubleFromScilab(4,&check))
  	{
		return true;
	}
	if (check==1)
	{
		/*sciprint("Gradient is not defined at the point [");
		for(i=0;i<numVars_;i++)
  		{
			if(i==numVars_-1)
				sciprint("%d",x[i]);
			else
				sciprint("%d,",x[i]);
  		}
		sciprint("], So the Point is skipped by IPopt during Iterations");*/
		return true;
	}	
	else
	{
		if(getDoubleMatrixFromScilab(3, &x0_rows, &x0_cols, &resg))
  		{	
			return true;
		}
		
  		for(i=0;i<numVars_;i++)
  		{
			grad_f[i]=resg[i];
  		}
		return true;
	}
}

// This method sets initial values for required vectors . For now we are assuming 0 to all values. 
bool minbndNLP::get_starting_point(Index n, bool init_x, Number* x,bool init_z, Number* z_L, Number* z_U,Index m, bool init_lambda,Number* lambda)
{
	Index i;
	for(i=0;i<n;i++)
		x[i]=NULL;

	return true;
}

/*
 * Return either the sparsity structure of the Hessian of the Lagrangian, 
 * or the values of the Hessian of the Lagrangian  for the given values for
 * x,lambda,obj_factor.
*/

bool minbndNLP::eval_h(Index n, const Number* x, bool new_x,Number obj_factor, Index m, const Number* lambda,bool new_lambda, Index nele_hess, Index* iRow,Index* jCol, Number* values)
{
 

	if (values==NULL)
	{
		Index idx=0;
		for (Index row = 0; row < numVars_; row++) 
		{
			for (Index col = 0; col <= row; col++)
			{
				iRow[idx] = row;
				jCol[idx] = col;
				idx++;
		  	}
		}
	}
	else 
	{
		int* gradhessptr=NULL;
		if(getFunctionFromScilab(2,&gradhessptr))
		{
			return 1;
		}  

		const Number *xNew=x;
		Index i;
  		double t=2;
	
  		createMatrixOfDouble(pvApiCtx, 3, 1, numVars_, xNew);
  		createScalarDouble(pvApiCtx, 4,t);
  		int positionFirstElementOnStackForScilabFunction = 3;
  		int numberOfRhsOnScilabFunction = 2;
  		int numberOfLhsOnScilabFunction = 2;
  		int pointerOnScilabFunction     = *gradhessptr;
		char name[20]="gradhess";
		
  		C2F(scistring)(&positionFirstElementOnStackForScilabFunction,name,
                                                               &numberOfLhsOnScilabFunction,
                                                               &numberOfRhsOnScilabFunction,(unsigned long)strlen(name));
                               
  		double* resh;
  		double check;  
  		int x0_rows,x0_cols;                           
  		
		if(getDoubleFromScilab(4,&check))
  		{
			return true;
		}
		if (check==1)
		{
			/*sciprint("Hessian is not defined at the point [");
			for(i=0;i<numVars_;i++)
  			{
				if(i=numVars_-1)
					sciprint("%d",x[i]);
				else
					sciprint("%d,",x[i]);
  			}
			sciprint("], So the Point is skipped by IPopt during Iterations");*/
			return true;
		}	
		else
		{
			if(getDoubleMatrixFromScilab(3, &x0_rows, &x0_cols, &resh))
			{
				sciprint("No results");
				return 1;
			}
			Index index=0;
			for (Index row=0;row < numVars_ ;++row)
			{
				for (Index col=0; col <= row; ++col)
				{
					values[index++]=obj_factor*(resh[numVars_*row+col]);
				}
			}
		}
		
	}
	
       	return true;
}


void minbndNLP::finalize_solution(SolverReturn status,Index n, const Number* x, const Number* z_L, const Number* z_U,Index m, const Number* g, const Number* lambda, Number obj_value,const IpoptData* ip_data,IpoptCalculatedQuantities* ip_cq)
{
	finalX_ = new double[n];
	for (Index i=0; i<n; i++) 
	{
    		 finalX_[i] = x[i];
	}
	
	finalZl_ = new double[n];
	for (Index i=0; i<n; i++) 
	{
    		 finalZl_[i] = z_L[i];
	}

	finalZu_ = new double[n];
	for (Index i=0; i<n; i++) 
	{
    		 finalZu_[i] = z_U[i];
	}

	finalObjVal_ = obj_value;
	status_ = status;

}


const double * minbndNLP::getX()
{	
	return finalX_;
}

double minbndNLP::getObjVal()
{	
	return finalObjVal_;
}

const double * minbndNLP::getZl()
{	
	return finalZl_;
}

const double * minbndNLP::getZu()
{	
	return finalZu_;
}

int minbndNLP::returnStatus()
{	
	return status_;
}

}

