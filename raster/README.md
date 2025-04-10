# Rasterization

## Inputs and Outputs
* Inputs: (t,x,y,z)
* Outputs: (t, $\lambda_1$, $\lambda_2$, $\lambda_3$)

where t is the triangle id; x, y, z are the triangle vertices after projecting. (z is needed for fragment shading and to solve triangle overlap issues)

For now there is no transparent id. Later we can add them and change inputs/outputs.

For each pixel, we check if it is inside triangles. Once it goes through all triangles, we give the outputs of which triangle the pixel (t) belongs, barycentric coordinates ($\lambda_1$, $\lambda_2$, $\lambda_3$)

## Some useful results
* Point P = $\lambda_1 V_1 + \lambda_2 V_2 + \lambda_3 V_3$ where $V_1, V_2, V_3$ are vertices of the triangle (projected one) then 
$$\lambda_1 + \lambda_2 + \lambda_3 = 1$$.

We use it to interpolate and estimate depth of this pixel and color accordingly.

* If $E_{AB}(P)$ is the edge function for edge defined by vertices $A, B$, at point $P$ then

$$ E_{AB}(P) = (x_P - x_A)(y_B - y_A) - (y_P - y_A)(x_B - x_A)$$

If you watch along A to B, then

$E(P) > 0$ if P is to the right side

$E(P) = 0$ if P is exactly on the line

$E(P) < 0$ if P is to the left side

So if point is inside the triange ABC, 

$$sgn(E_{AB}(P)) = sgn(E_{BC}(P)) = sgn(E_{CA}(P))$$

* The barycentric coordinates are computed as

$$ \lambda_A = \frac{E_{BC}(P)}{E_{BC}(A)}, \lambda_B = \frac{E_{CA}(P)}{E_{CA}(B)}, \lambda_C = \frac{E_{AB}(P)}{E_{AB}(C)}$$
$$ E_{AB}(C) = E_{BC}(A) = E_{CA}(B) = 2 \times Area(ABC) $$

* The correct approach is to linearly interpolate the vertices inverse z-coordinates using barycentric coordinates.

$$ \frac{1}{z_P} = \frac{1}{z_A}\lambda_A + \frac{1}{z_B}\lambda_B + \frac{1}{z_C}\lambda_C $$

$$ \frac{1}{z_P} = (\frac{1}{z_A}-\frac{1}{z_C})\lambda_A + (\frac{1}{z_B}-\frac{1}{z_C})\lambda_B + \frac{1}{z_C}$$

The terms $\frac{1}{z_A}-\frac{1}{z_C}$ and $\frac{1}{z_B}-\frac{1}{z_C}$ are precomputed. So two additions and two multiplications.

First calculate the $z_P$ of this triangle and then similarly calculate for next triangle, the triangle id is updated if new z is lower(basically if lower depth) else it remains same and goes to next triangle and so on.

* Barycentric coordinates remain constant along lines parallel to an edge

## Some tips and tricks

* Since we go pixel by pixel vertically, we can reduce number of computations in $E_{AB}$.

$$ E_{AB}' = (x_P - x_A)(y_B - y_A) - ((y_P-k) - y_A)(x_B - x_A) = E_{AB} + k * (x_B - x_A) $$

Similarly when one vertical processing is done, we can keep y constant and go to increase x by 1 unit (=k).

$$ E_{AB}' = ((x_P+k) - x_A)(y_B - y_A) - (y_P - y_A)(x_B - x_A) = E_{AB} + k * (y_B - y_A) $$

* In calculation of barycentric coordinates, the denominator is related to area of triangle. So we dont need to compute $\frac{1}{E_{AB}(C)}, \frac{1}{E_{BC}(A)}, \frac{1}{E_{CA}(B)}$ multiple times.

* For a depth z, we don't store z but it is stored as $\frac{1}{z}$ to avoid division.