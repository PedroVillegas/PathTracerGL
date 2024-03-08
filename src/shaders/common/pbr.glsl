// Following PBR as described in https://www.google.github.io/filament/Filament.html

float D_GGX(float NoH, float roughness)
{
	/* 
	* Normal Distribution Function D (GGX Distribution)
	* Models the distribution of the microfacets
	*/
	float a = NoH * roughness;
	float k = roughness / (1.0 - NoH * NoH + a * a);
	return k * k * (ONE_PI);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float roughness)
{
	/*
	* Visibility Term G / V
	* Models the visibility (or occlusion or shadow-masking) of the microfacets
	* Input roughness is expected as pre-mapped
	*/
	float a2 = roughness * roughness;
	float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
	float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
	return 0.5 / (GGXV + GGXL);
}

float V_SmithGGXMaskingShadowing(float NoV, float NoL, float roughness)
{
	// Non height-correlated masking-shadowing function
	float a2 = roughness * roughness;
    float denomA = NoV * sqrt(a2 + (1.0 - a2) * NoL * NoL);
    float denomB = NoL * sqrt(a2 + (1.0 - a2) * NoV * NoV);

    return 2.0 * NoL * NoV / (denomA + denomB);
}

float V_SmithGGXMasking(float NoV, float roughness)
{
	float a2 = roughness * roughness;
    float denomC = sqrt(a2 + (1.0 - a2) * NoV * NoV) + NoV;

    return 2.0 * NoV / denomC;
}


vec3 F_Schlick(float u, vec3 f0 /*, float f90 */)
{
	/*
	* Fresnel Term F
	* Defines how light reflects or refracts at the interface between two media
	* f0  : specular reflectance at normal incidence
	* f90 : reflectance at grazing angles (set to 1.0 for realism)
	*/
	// return f0 + (vec3(f90) - f0) * pow(1.0 - u, 5.0);
	float f = pow(1.0 - u, 5.0);
	return f + f0 * (1.0 - f);
}

float F_Schlick(float u, float f0, float f90)
{
	return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float F_Dielectric(float cosThetaI, float eta)
{
    float sinThetaTSq = eta * eta * (1.0 - cosThetaI * cosThetaI);

    // Total internal reflection
    if (sinThetaTSq > 1.0)
        return 1.0;

    float cosThetaT = sqrt(max(1.0 - sinThetaTSq, 0.0));

    float rs = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);
    float rp = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);

    return 0.5 * (rs * rs + rp * rp);
}

float Fd_Lambert()
{	
	return ONE_PI;
}

float Fd_Burley(float NoV, float NoL, float LoH, float roughness)
{
	/*
	* Alternative diffuse BRDF takes roughness into account and creates
	* retro-reflections at grazing angles
	*/ 
	float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
	float lightScatter = F_Schlick(NoL, 1.0, f90);
	float viewScatter = F_Schlick(NoV, 1.0, f90);
	return lightScatter * viewScatter * (ONE_PI);
}

// From Pixar - https://graphics.pixar.com/library/OrthonormalB/paper.pdf
void Basis(in vec3 n, out vec3 b1, out vec3 b2) 
{
    if (n.z < 0.)
	{
        float a = 1.0 / (1.0 - n.z);
        float b = n.x * n.y * a;
        b1 = vec3(1.0 - n.x * n.x * a, -b, n.x);
        b2 = vec3(b, n.y * n.y*a - 1.0, -n.y);
    }
    else
	{
        float a = 1.0 / (1.0 + n.z);
        float b = -n.x * n.y * a;
        b1 = vec3(1.0 - n.x * n.x * a, b, -n.x);
        b2 = vec3(b, 1.0 - n.y * n.y * a, -n.y);
    }
}

// https://jcgt.org/published/0007/04/01/paper.pdf
// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
vec3 SampleGGXVNDF(vec3 Ve, float alpha_x, float alpha_y, float U1, float U2)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	vec3 Vh = normalize(vec3(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));

	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	vec3 T1 = lensq > 0 ? vec3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : vec3(1,0,0);
	vec3 T2 = cross(Vh, T1);

	// Section 4.2: parameterization of the projected area
	float r = sqrt(U1);
	float phi = 2.0 * PI * U2;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + Vh.z);
	t2 = (1.0 - s)*sqrt(1.0 - t1*t1) + s*t2;

	// Section 4.3: reprojection onto hemisphere
	vec3 Nh = t1*T1 + t2*T2 + sqrt(max(0.0, 1.0 - t1*t1 - t2*t2))*Vh;

	// Section 3.4: transforming the normal back to the ellipsoid configuration
	vec3 Ne = normalize(vec3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0, Nh.z)));
	return Ne;
}

float Luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 toWorld(vec3 x, vec3 y, vec3 z, vec3 v)
{
    return v.x*x + v.y*y + v.z*z;
}

vec3 toLocal(vec3 x, vec3 y, vec3 z, vec3 v)
{
    return vec3(dot(v, x), dot(v, y), dot(v, z));
}

// https://schuttejoe.github.io/post/ggximportancesamplingpart2/
vec3 EvalIndirectBSDF(inout Ray ray, Payload shadingPoint, out float pdf, inout bool lastBounceSpecular)
{
	/*
	* View Vector	   : v || wo
	* Light Vector     : l || wi
	* Half Vector      : h || wm
	* Geometric Normal : n || wg
	*/

	vec3 l;
	vec3 v = -ray.direction;
	vec3 n = shadingPoint.normal;
	vec3 position	   = shadingPoint.position;
	vec3 albedo		   = shadingPoint.mat.albedo;
	float transmission = shadingPoint.mat.transmission;
	float ior          = shadingPoint.mat.ior;
	float metallic	   = shadingPoint.mat.metallic;
	float roughness    = shadingPoint.mat.roughness;

	float rand1 = Randf01();
	float rand2 = Randf01();

	// Sample a microfacet normal from GGX distribution
	vec3 t, b;
    Basis(n, t, b);
    vec3 Ve = toLocal(t, b, n, v);
    vec3 h = SampleGGXVNDF(Ve, roughness, roughness, rand1, rand2);
    if (h.z < 0.0)
        h = -h;
    h = toWorld(t, b, n, h);

	// Determine the brdf to sample with fresnel
	float VoH = dot(v, h);
	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 F = F_Schlick(VoH, f0);
	float n_1 = shadingPoint.fromInside ? ior : 1.0;
    float n_2 = shadingPoint.fromInside ? 1.0 : ior;
	float dF = F_Dielectric(abs(VoH), n_1 / n_2);
    
	// Lobe weight probability
	pdf = 1.0;
	float dWeight = (1.0 - metallic) * (1.0 - transmission);
	float sWeight = Luma(F);
	float tWeight = transmission * (1.0 - metallic) * (1.0 - dF);
	float invW    = 1.0 / (dWeight + sWeight + tWeight);
	dWeight *= invW;
	sWeight *= invW;
	tWeight *= invW;
    
	// cdf
	float cdf[3];
	cdf[0] = dWeight;
	cdf[1] = cdf[0] + sWeight;
	cdf[2] = cdf[1] + tWeight;
    
	vec3 brdf = vec3(0.0);
	if (rand1 < cdf[0]) // Diffuse
	{
		l = SampleCosineHemisphere(rand1, rand2, n);
		h = normalize(l + v);

		ray.direction = l;
		ray.origin = position + n * EPS;
		
		pdf *= dWeight;
		vec3 Fd = albedo ;
		brdf = Fd * abs(dot(n, l));
	} 
	else if (rand1 < cdf[1]) // Specular
	{
		lastBounceSpecular = true;
		l = reflect(-v, h);

		ray.direction = l;
		ray.origin = position + n * EPS;
        
		float NoL = dot(n, l);
		float NoV = dot(n, v);
		if (NoL <= 0.0 || NoV <= 0.0)
			return vec3(0.0);
        
		float G1 = V_SmithGGXMasking(NoV, roughness);
		float G2 = V_SmithGGXMaskingShadowing(NoV, NoL, roughness);
		if (G1 <= 0.0) return vec3(0.0);

		// float D  = D_GGX(NoH, roughness);
		// pdf = G1 * VoH * D / NoV * 4 * VoH;

		/*
		* Note: the pdf = G1 after cancelling terms as follows
		*
		*		 F * G2 * D * NoL	  1		  F * G2 * D * NoL	    NoV * 4 * VoH		F * G2
		* Fs =  -----------------  * ---  =  -----------------  *  ---------------  =  --------
		*		  4 * NoL * NoV		 pdf	   4 * NoL * NoV		G1 * VoH * D		  G1
		*
		*/		

		pdf *= sWeight;
		// Move G1 to Fs
		vec3 Fs = F * (G2 / G1);
		brdf = Fs;
	}
	else //if (rand1 < cdf[2]) // Transmission
	{
		lastBounceSpecular = true;
		float eta = shadingPoint.fromInside ? ior : (1.0 / ior);
		l = refract(-v, h, eta);

		ray.direction = l;
		ray.origin = position - n * EPS;

		h = normalize(v + l * eta);

		float NoL = Saturate(abs(dot(n, l)));
        float NoV = Saturate(abs(dot(n, v)));
		float LoH = Saturate(abs(dot(l, h)));
		float VoH = Saturate(abs(dot(v, h)));

		float iorV = shadingPoint.fromInside ? ior : 1.0;
        float iorL = !shadingPoint.fromInside ? ior : 1.0;
		
		// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf - Eq. 21
		//float D = D_GGX(NoH, roughness);
		float G1 = V_SmithGGXMasking(NoV, roughness);
		float G2 = V_SmithGGXMaskingShadowing(NoV, NoL, roughness);
		float denom = iorL * LoH + iorV * VoH;
		denom *= denom;
		if (denom <= EPS || G1 <= 0.0 || dF > 1.0) return vec3(0.0);
		//float jacobian = LoH / denom;
		//pdf = G1 * D * max(0.0, dot(v, h)) * jacobian / NoV;
		//vec3 Ft = vec3(1.0) * jacobian * VoH * iorV * iorV * (1.0 - dF) * G2 * D / (denom * NoL * NoV);

		/*
		* Note: the pdf = G1 after cancelling terms as follows
		*
		*		 J * VoH * iorV * iorV * (1.0 - F) * G2 * D		1
		* Ft =  -------------------------------------------- * ---
		*						NoL * NoV		               pdf
		*
		*        J * VoH * iorV * iorV * (1.0 - F) * G2 * D			   NoV
		*	 =  -------------------------------------------- * ------------------
		*						NoL * NoV						G1 * D * VoH * J
		*
		*        iorV * iorV * (1.0 - F) * G2
		*	 =  ------------------------------
		*					 G1
		*
		*/

		pdf *= tWeight;
		// Move G1 to Ft
		vec3 Ft = vec3(1.0) * iorV * iorV * (1.0 - dF) * G2 / G1;
		brdf = Ft;
	}
	pdf = 1.0;
	return brdf;
}

vec3 EvalBSDF(Ray ray, Payload shadingPoint, vec3 l, out float pdf)
{
	/*
	* View Vector	   : v || wo
	* Light Vector     : l || wi
	* Half Vector      : h || wm
	* Geometric Normal : n || wg
	*/

	vec3 v = -ray.direction;
	vec3 n = shadingPoint.normal;
	vec3 albedo		= shadingPoint.mat.albedo;
	float metallic  = shadingPoint.mat.metallic;
	float roughness = shadingPoint.mat.roughness;
	float transmission = shadingPoint.mat.transmission;

	float rand1 = Randf01();
	float rand2 = Randf01();

	vec3 h = normalize(v + l);
	
	float NoL = dot(n, l);
	float NoV = dot(n, v);
	if (NoL <= 0.0 || NoV <= 0.0)
		return vec3(0.0);
	float LoH = Saturate(dot(l, h));

	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 F = F_Schlick(LoH, f0);
	float G1 = V_SmithGGXMasking(NoV, roughness);
	float G2 = V_SmithGGXMaskingShadowing(NoV, NoL, roughness);
	pdf = 1.0;

	/*
	* Note: the pdf is 1.0 after cancelling terms as follows
	*
	*		 F * G2 * D * NoL	  1		  F * G2 * D * NoL	    NoV * 4 * VoH		F * G2
	* Fs =  -----------------  * ---  =  -----------------  *  ---------------  =  --------
	*		  4 * NoL * NoV		 pdf	   4 * NoL * NoV		G1 * VoH * D		  G1
	*
	*/	

	vec3 Fs = F * (G2 / G1) * (1.0 - transmission) * (1.0 - metallic);
	vec3 Fd = albedo * (1.0 - F) * (1.0 - transmission) * (1.0 - metallic);
	vec3 brdf = Fd + Fs;

	return brdf;
}
