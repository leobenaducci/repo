

bool FastIntersectAABB(vec3 rpos, vec3 invrdir, vec3 vmin, vec3 vmax, float lastIntersectTime)
{
	vec3 tA = (vmin - rpos) * invrdir;
	vec3 tB = (vmax - rpos) * invrdir;
	vec3 Mins = min(tA, tB);
	vec3 Maxs = max(tA, tB);

	float fMax = max(max(Mins.x, Mins.y), Mins.z);
	float fMin = min(min(Maxs.x, Maxs.y), Maxs.z);
	
	return max(0.f, fMax) < min(max(0.f, fMin), lastIntersectTime);
}

bool IntersectTriangle(vec3 orig, vec3 dir, vec3 A, vec3 B, vec3 C, out float U, out float V, out vec3 P)
{
	const vec3 v0 = A;

	const vec3 u = B - v0;
	const vec3 v = C - v0;
	const vec3 n = cross(u, v);
	const float b = dot(n, dir);

	const vec3 w0 = orig - v0;
	const float a = -dot(n, w0);

	// get intersect point of ray with triangle plane
	const float r = a / b;
	const vec3 I = orig + r * dir;
	const float uu = dot(u, u);
	const float uv = dot(u, v);
	const float vv = dot(v, v);
	const vec3 w = I - v0;
	const float wu = dot(w, u);
	const float wv = dot(w, v);
	const float invD = 1.0 / (uv * uv - uu * vv);

	// get and test parametric coords
	U = (uv * wv - vv * wu) * invD;
	V = (uv * wu - uu * wv) * invD;
	P = I;

	return r >= 0.0 && !(min(r, min(U, V)) < 0.0 || max(U, U + V) > 1.0);
}

void IntersectSceneBvh(inout Context_s Context, int RayType)
{
	Object_s Obj;
	MeshShape_s Shape;
	BvhNode_s* Tree = Scene.Tree;
	vec3 PrevRayOrigin, PrevRayDir;

	int Node = 0, SceneNode = 0;
	do
	{
		const BvhNode_s BvhNode = Tree[Node];
		ivec2 ChildsNext = ivec2(floatBitsToInt(BvhNode.AABBMin.w), floatBitsToInt(BvhNode.AABBMax.w));
		if (FastIntersectAABB(Context.Ray.Origin, 1.0 / Context.Ray.Direction, BvhNode.AABBMin.xyz, BvhNode.AABBMax.xyz, Context.Result.Distance))
		{
			if (ChildsNext.x > 0)  // is node?
			{
				Node++;
				continue;
			}
			else if(SceneNode == 0) // is scene tree?
			{
				Obj = Scene.Objects[-(ChildsNext.x + 1)];
				Shape = Meshes[Obj.MeshShape.x].Shapes[Obj.MeshShape.y];
				Tree = Shape.BvhTree;

				PrevRayOrigin = Context.Ray.Origin;
				PrevRayDir = Context.Ray.Direction;

				Context.Ray.Origin = (Obj.InverseTransform * vec4(Context.Ray.Origin, 1.0)).xyz;
				Context.Ray.Direction = mat3(Obj.InverseTransform) * vec3(Context.Ray.Direction);

				//last one is always 0
				SceneNode = Node + 1;
				Node = 0;
				continue;
			}
			else // is mesh tree leaf?
			{
				vec3 TempPos;
				int polyIdx = -(ChildsNext.x + 1);

				vec4 A = Shape.Triangles[polyIdx * 3 + 0];
				vec4 B = Shape.Triangles[polyIdx * 3 + 1];
				vec4 C = Shape.Triangles[polyIdx * 3 + 2];

				float U, V;
				if (IntersectTriangle(Context.Ray.Origin, Context.Ray.Direction, A.xyz, B.xyz, C.xyz, U, V, TempPos))
				{
					RayResult_s Temp;

					vec3 N = vec3(A.w, B.w, C.w);
#if RAYTRACE_HAS_RESULT_FACE_CULLING
					if (dot(N, Context.Ray.Direction) * Context.FaceCullingValue >= 0.0)
#endif
					{
						Vertex_s vertices[3] = { Shape.Vertices[polyIdx * 3 + 0], Shape.Vertices[polyIdx * 3 + 1], Shape.Vertices[polyIdx * 3 + 2] };

						TempPos = (Obj.Transform * vec4(TempPos, 1.0)).xyz;

#if RAYTRACE_HAS_RESULT_POSITION
						Temp.Pos = TempPos;
#endif

#if RAYTRACE_HAS_RESULT_NORMAL
						Temp.Normal = vertices[1].Normal.xyz * U + vertices[2].Normal.xyz * V + vertices[0].Normal.xyz * (1.0 - (U + V));
						Temp.Normal = mat3(Obj.Transform) * Temp.Normal.xyz;
#endif

#if RAYTRACE_HAS_RESULT_MATERIAL
						Temp.TexCoord = vertices[1].TexCoord.xy * U + vertices[2].TexCoord.xy * V + vertices[0].TexCoord.xy * (1.0 - (U + V));
#endif
						Temp.Distance = length(TempPos - PrevRayOrigin);

#if RAYTRACE_HAS_ANY_HIT || !RAYTRACE_HAS_CLOSEST_HIT
						if (AnyHit(Context.Ray, Temp, Obj.Material, RayType, Context.Payload) || !RAYTRACE_HAS_CLOSEST_HIT)
						{
							Context.bValidAny = true;
							CopyTempResult(Context, Temp, Obj);
							return;
						}
#endif

#if RAYTRACE_HAS_CLOSEST_HIT
						if (Temp.Distance < Context.Result.Distance)
						{
							Context.bValidClosest = true;
							CopyTempResult(Context, Temp, Obj);
						}
#endif
					}
				}
			}
		}

		Node = ChildsNext.y;

		if (Node == 0 && SceneNode != 0)
		{
			Node = SceneNode;

			Context.Ray.Origin = PrevRayOrigin;
			Context.Ray.Direction = PrevRayDir;

			Tree = Scene.Tree;
			SceneNode = 0;
			continue;
		}
	} while (Node != 0);

	if (Context.bValidClosest)
	{
#if RAYTRACE_HAS_RESULT_MATERIAL
		SampleTextures(Context);
#endif
#if RAYTRACE_HAS_RESULT_NORMAL
		Context.Result.Normal = normalize(Context.Result.Normal);
#endif
	}
}
