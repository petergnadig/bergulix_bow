select distinct
id_m_head
mdate,
bow.bow_description,
person.person_name,
gps_lat,
gps_lon,
group_concat(imu_serial separator ',') as ImuS
from m_head 
left join m_imu on fk_id_m_head=id_m_head
left join bow on bow.id_bow=m_head.fk_bow_id
left join person on person.id_person=m_head.fk_person_id
group by 1
having mdate=112

